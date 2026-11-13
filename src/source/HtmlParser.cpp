#include "HtmlParser.h"
#include <QRegularExpression>
#include <QSet>
#include <QDebug>

struct AttributeFilter {
    QString name;
    QString op;
    QString value;
};

struct CompoundSelector {
    QString tag;
    QString id;
    QStringList classes;
    QList<AttributeFilter> attributes;
    QString containsText;
    char combinator = ' '; // ' ' for descendant, '>' for child
};

static QString getNodeAttr(const GumboNode* node, const QString& attrName) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return QString();
    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attrName.toUtf8().constData());
    if (attr) return QString::fromUtf8(attr->value);
    return QString();
}

static bool hasNodeAttr(const GumboNode* node, const QString& attrName) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return false;
    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attrName.toUtf8().constData());
    return attr != nullptr;
}

static QString getNodeTagName(const GumboNode* node) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return QString();
    if (node->v.element.tag != GUMBO_TAG_UNKNOWN) {
        const char* name = gumbo_normalized_tagname(node->v.element.tag);
        if (name) return QString::fromUtf8(name).toLower();
    }
    GumboStringPiece piece = node->v.element.original_tag;
    if (piece.data && piece.length > 0) {
        int i = 0;
        while (i < piece.length && (piece.data[i] == '<' || piece.data[i] == '/' || piece.data[i] == ' ' || piece.data[i] == '\t')) {
            i++;
        }
        int start = i;
        while (i < piece.length && piece.data[i] != '>' && piece.data[i] != '/' && piece.data[i] != ' ' && piece.data[i] != '\t' && piece.data[i] != '\n' && piece.data[i] != '\r') {
            i++;
        }
        if (i > start) {
            return QString::fromUtf8(piece.data + start, i - start).toLower();
        }
    }
    return QString();
}

static QString getNodeText(const GumboNode* node) {
    if (!node) return QString();
    if (node->type == GUMBO_NODE_TEXT || node->type == GUMBO_NODE_CDATA) {
        return QString::fromUtf8(node->v.text.text);
    }
    if (node->type == GUMBO_NODE_ELEMENT) {
        if (node->v.element.tag == GUMBO_TAG_SCRIPT || node->v.element.tag == GUMBO_TAG_STYLE) return QString();
        QString result;
        const GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            result += getNodeText(static_cast<const GumboNode*>(children->data[i]));
        }
        return result;
    }
    return QString();
}

static QString serializeGumboNode(const GumboNode* node, bool innerOnly = false) {
    if (!node) return QString();
    if (node->type == GUMBO_NODE_TEXT || node->type == GUMBO_NODE_CDATA) {
        return QString::fromUtf8(node->v.text.text);
    }
    if (node->type == GUMBO_NODE_ELEMENT) {
        QString tag = getNodeTagName(node);
        QString inner;
        const GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            inner += serializeGumboNode(static_cast<const GumboNode*>(children->data[i]), false);
        }

        if (innerOnly) return inner;

        QString attrs;
        const GumboVector* attrList = &node->v.element.attributes;
        for (unsigned int i = 0; i < attrList->length; ++i) {
            const GumboAttribute* attr = static_cast<const GumboAttribute*>(attrList->data[i]);
            attrs += QString(" %1=\"%2\"").arg(QString::fromUtf8(attr->name), QString::fromUtf8(attr->value));
        }

        static const QSet<QString> voidTags = {
            "area", "base", "br", "col", "embed", "hr", "img", "input",
            "link", "meta", "param", "source", "track", "wbr"
        };

        if (voidTags.contains(tag) && inner.isEmpty()) {
            return QString("<%1%2 />").arg(tag, attrs);
        }
        return QString("<%1%2>%3</%1>").arg(tag, attrs, inner);
    }
    return QString();
}

static bool matchesCompound(const GumboNode* node, const CompoundSelector& sel) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return false;

    // Check tag
    if (!sel.tag.isEmpty() && sel.tag != "*") {
        if (getNodeTagName(node) != sel.tag.toLower()) return false;
    }

    // Check ID
    if (!sel.id.isEmpty()) {
        if (getNodeAttr(node, "id") != sel.id) return false;
    }

    // Check classes
    if (!sel.classes.isEmpty()) {
        QString classAttr = getNodeAttr(node, "class");
        QStringList nodeClasses = classAttr.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const QString& cls : sel.classes) {
            if (!nodeClasses.contains(cls)) return false;
        }
    }

    // Check attributes
    for (const AttributeFilter& af : sel.attributes) {
        if (!hasNodeAttr(node, af.name)) return false;
        if (!af.op.isEmpty()) {
            QString actual = getNodeAttr(node, af.name);
            if (af.op == "=" && actual != af.value) return false;
            if (af.op == "*=" && !actual.contains(af.value)) return false;
            if (af.op == "^=" && !actual.startsWith(af.value)) return false;
            if (af.op == "$=" && !actual.endsWith(af.value)) return false;
            if (af.op == "~=" && !actual.split(QRegularExpression("\\s+")).contains(af.value)) return false;
        }
    }

    // Check containsText
    if (!sel.containsText.isEmpty()) {
        if (!getNodeText(node).contains(sel.containsText, Qt::CaseInsensitive)) return false;
    }

    return true;
}

static void collectDescendants(const GumboNode* node, const CompoundSelector& sel, QList<const GumboNode*>& results) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return;

    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        const GumboNode* child = static_cast<const GumboNode*>(children->data[i]);
        if (child && child->type == GUMBO_NODE_ELEMENT) {
            if (matchesCompound(child, sel)) {
                results.append(child);
            }
            collectDescendants(child, sel, results);
        }
    }
}

static void collectChildren(const GumboNode* node, const CompoundSelector& sel, QList<const GumboNode*>& results) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return;

    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        const GumboNode* child = static_cast<const GumboNode*>(children->data[i]);
        if (child && child->type == GUMBO_NODE_ELEMENT) {
            if (matchesCompound(child, sel)) {
                results.append(child);
            }
        }
    }
}

static CompoundSelector parseCompoundToken(const QString& token, char combinator) {
    CompoundSelector sel;
    sel.combinator = combinator;

    QString s = token.trimmed();

    // Parse attributes: e.g. [attr*=val]
    QRegularExpression attrRegex("\\[([a-zA-Z0-9_:-]+)(?:([*^$~]?=)(?:\"([^\"]*)\"|'([^']*)'|([^\\s\\]]+)))?\\]");
    auto it = attrRegex.globalMatch(s);
    while (it.hasNext()) {
        auto match = it.next();
        AttributeFilter af;
        af.name = match.captured(1);
        af.op = match.captured(2);
        if (!match.captured(3).isEmpty()) af.value = match.captured(3);
        else if (!match.captured(4).isEmpty()) af.value = match.captured(4);
        else af.value = match.captured(5);
        sel.attributes.append(af);
    }
    s.remove(attrRegex);

    // Parse pseudo :contains(text)
    QRegularExpression containsRegex(":contains\\((?:\"([^\"]*)\"|'([^']*)'|([^)]+))\\)");
    auto cMatch = containsRegex.match(s);
    if (cMatch.hasMatch()) {
        sel.containsText = !cMatch.captured(1).isEmpty() ? cMatch.captured(1) : (!cMatch.captured(2).isEmpty() ? cMatch.captured(2) : cMatch.captured(3));
        s.remove(containsRegex);
    }

    // Parse ID
    QRegularExpression idRegex("#([a-zA-Z0-9_:-]+)");
    auto idMatch = idRegex.match(s);
    if (idMatch.hasMatch()) {
        sel.id = idMatch.captured(1);
        s.remove(idRegex);
    }

    // Parse classes
    QRegularExpression classRegex("\\.([a-zA-Z0-9_:-]+)");
    auto classIt = classRegex.globalMatch(s);
    while (classIt.hasNext()) {
        auto cm = classIt.next();
        sel.classes.append(cm.captured(1));
    }
    s.remove(classRegex);

    // Remaining must be tag name
    s = s.trimmed();
    if (!s.isEmpty()) {
        sel.tag = s;
    }

    return sel;
}

static QList<CompoundSelector> parseSelectorChain(const QString& chainStr) {
    QList<CompoundSelector> chain;
    QString s = chainStr.trimmed();

    s.replace(QRegularExpression("\\s*>\\s*"), " > ");
    QStringList parts = s.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    char currentCombinator = ' ';
    for (int i = 0; i < parts.size(); ++i) {
        if (parts[i] == ">") {
            currentCombinator = '>';
            continue;
        }
        chain.append(parseCompoundToken(parts[i], currentCombinator));
        currentCombinator = ' ';
    }
    return chain;
}

static QList<const GumboNode*> evaluateChain(const GumboNode* root, const QList<CompoundSelector>& chain) {
    if (chain.isEmpty() || !root) return {};

    QList<const GumboNode*> current;
    collectDescendants(root, chain[0], current);

    for (int i = 1; i < chain.size(); ++i) {
        const CompoundSelector& sel = chain[i];
        QList<const GumboNode*> next;
        for (const GumboNode* parent : current) {
            if (sel.combinator == '>') {
                collectChildren(parent, sel, next);
            } else {
                collectDescendants(parent, sel, next);
            }
        }
        // Deduplicate
        QList<const GumboNode*> dedup;
        for (const GumboNode* n : next) {
            if (!dedup.contains(n)) dedup.append(n);
        }
        current = dedup;
    }
    return current;
}

// HtmlElement implementation
HtmlElement::HtmlElement(std::shared_ptr<GumboDocumentState> doc, const GumboNode* node, QObject *parent)
    : QObject(parent), m_doc(doc), m_node(node)
{
}

QString HtmlElement::text() const
{
    return getNodeText(m_node).trimmed();
}

QString HtmlElement::innerHtml() const
{
    return serializeGumboNode(m_node, true);
}

QString HtmlElement::outerHtml() const
{
    return serializeGumboNode(m_node, false);
}

QString HtmlElement::tagName() const
{
    return getNodeTagName(m_node);
}

QString HtmlElement::attr(const QString& name) const
{
    return getNodeAttr(m_node, name);
}

bool HtmlElement::hasAttr(const QString& name) const
{
    return hasNodeAttr(m_node, name);
}

QVariantList HtmlElement::select(const QString& selector)
{
    QList<HtmlElement*> list = HtmlParser::query(m_doc, m_node, selector);
    QVariantList result;
    result.reserve(list.size());
    for (HtmlElement* el : list) {
        el->setParent(this);
        result.append(QVariant::fromValue<QObject*>(el));
    }
    return result;
}

QObject* HtmlElement::first(const QString& selector)
{
    QList<HtmlElement*> list = HtmlParser::query(m_doc, m_node, selector);
    if (!list.isEmpty()) {
        HtmlElement* firstEl = list.first();
        firstEl->setParent(this);
        for (int i = 1; i < list.size(); ++i) {
            delete list[i];
        }
        return firstEl;
    }
    return nullptr;
}

// HtmlParser implementation
HtmlParser::HtmlParser(QObject *parent)
    : QObject(parent)
{
}

QList<HtmlElement*> HtmlParser::query(std::shared_ptr<GumboDocumentState> doc, const GumboNode* root, const QString& selector)
{
    if (!doc || !root) return {};

    QList<HtmlElement*> elements;
    QList<const GumboNode*> matchedNodes;

    QStringList subQueries = selector.split(",", Qt::SkipEmptyParts);
    for (const QString& sub : subQueries) {
        auto chain = parseSelectorChain(sub);
        auto nodes = evaluateChain(root, chain);
        for (const GumboNode* n : nodes) {
            if (!matchedNodes.contains(n)) {
                matchedNodes.append(n);
            }
        }
    }

    elements.reserve(matchedNodes.size());
    for (const GumboNode* n : matchedNodes) {
        // Parent each element to the doc's first element so it survives GC
        // but gets cleaned up when the HtmlParser or its parent engine destructs.
        elements.append(new HtmlElement(doc, n, nullptr));
    }
    return elements;
}

QVariantList HtmlParser::select(const QString& html, const QString& selector)
{
    auto doc = std::make_shared<GumboDocumentState>();
    doc->buffer = html.toUtf8();
    doc->output = gumbo_parse_with_options(&kGumboDefaultOptions, doc->buffer.constData(), doc->buffer.size());
    if (!doc->output || !doc->output->root) return QVariantList();

    QList<HtmlElement*> list = query(doc, doc->output->root, selector);
    QVariantList result;
    result.reserve(list.size());
    for (HtmlElement* el : list) {
        el->setParent(this);
        result.append(QVariant::fromValue<QObject*>(el));
    }
    return result;
}

QObject* HtmlParser::first(const QString& html, const QString& selector)
{
    auto doc = std::make_shared<GumboDocumentState>();
    doc->buffer = html.toUtf8();
    doc->output = gumbo_parse_with_options(&kGumboDefaultOptions, doc->buffer.constData(), doc->buffer.size());
    if (!doc->output || !doc->output->root) return nullptr;

    QList<HtmlElement*> list = query(doc, doc->output->root, selector);
    if (!list.isEmpty()) {
        HtmlElement* firstEl = list.first();
        firstEl->setParent(this);
        for (int i = 1; i < list.size(); ++i) {
            delete list[i];
        }
        return firstEl;
    }
    return nullptr;
}

QObject* HtmlParser::parse(const QString& html)
{
    auto doc = std::make_shared<GumboDocumentState>();
    doc->buffer = html.toUtf8();
    doc->output = gumbo_parse_with_options(&kGumboDefaultOptions, doc->buffer.constData(), doc->buffer.size());
    if (!doc->output || !doc->output->root) return nullptr;

    return new HtmlElement(doc, doc->output->root);
}
