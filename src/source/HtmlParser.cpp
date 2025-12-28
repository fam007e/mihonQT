#include "HtmlParser.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStack>

HtmlElement::HtmlElement(const QString& tag, const QString& attrStr, const QString& inner, QObject *parent)
    : QObject(parent), m_tagName(tag), m_attributes(attrStr), m_innerHtml(inner)
{
}

QString HtmlElement::text() const
{
    // Strip tags from innerHtml
    QString copy = m_innerHtml;
    // Remove <script>...</script> and <style>...</style> content first
    copy.remove(QRegularExpression("<script[^>]*>.*?</script>", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption));
    copy.remove(QRegularExpression("<style[^>]*>.*?</style>", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption));

    // Remove tags
    copy.remove(QRegularExpression("<[^>]*>"));

    // Decode entities (basic ones)
    copy.replace("&nbsp;", " ");
    copy.replace("&amp;", "&");
    copy.replace("&lt;", "<");
    copy.replace("&gt;", ">");
    copy.replace("&quot;", "\"");
    copy.replace("&#39;", "'");

    return copy.trimmed();
}

QString HtmlElement::outerHtml() const
{
    return QString("<%1%2>%3</%1>").arg(m_tagName, m_attributes.isEmpty() ? "" : " " + m_attributes, m_innerHtml);
}

QString HtmlElement::attr(const QString& name)
{
    // Regex to find attribute value
    // name="value" or name='value' or name=value
    QRegularExpression regex(QString("%1\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s>]*))").arg(QRegularExpression::escape(name)), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regex.match(m_attributes);
    if (match.hasMatch()) {
        if (!match.captured(1).isEmpty()) return match.captured(1);
        if (!match.captured(2).isEmpty()) return match.captured(2);
        return match.captured(3);
    }
    return QString();
}

QVariantList HtmlElement::select(const QString& selector)
{
    QList<HtmlElement*> list = HtmlParser::parse(m_innerHtml, selector);
    QVariantList vList;
    for (HtmlElement* el : list) {
        // Do not set parent. Let JS engine take ownership.
        // el->setParent(this);
        vList.append(QVariant::fromValue<QObject*>(el));
    }
    return vList;
}

QObject* HtmlElement::first(const QString& selector)
{
    QList<HtmlElement*> list = HtmlParser::parse(m_innerHtml, selector);
    if (!list.isEmpty()) {
        HtmlElement* el = list.first();
        // Do not set parent
        // el->setParent(this);
        // Delete others
        for (int i=1; i<list.size(); ++i) delete list[i];
        return el;
    }
    return nullptr;
}

HtmlParser::HtmlParser(QObject *parent) : QObject(parent)
{
}

QVariantList HtmlParser::select(const QString& html, const QString& selector)
{
    QList<HtmlElement*> list = parse(html, selector);
    QVariantList vList;
    for (HtmlElement* el : list) {
        // el->setParent(this); // Manage memory - changed to let JS own it
        vList.append(QVariant::fromValue<QObject*>(el));
    }
    return vList;
}

QObject* HtmlParser::first(const QString& html, const QString& selector)
{
    QList<HtmlElement*> list = parse(html, selector);
    if (!list.isEmpty()) {
        HtmlElement* el = list.first();
        // Do not set parent
        // el->setParent(this);
        // Delete others
        for (int i=1; i<list.size(); ++i) delete list[i];
        return el;
    }
    return nullptr;
}

QObject* HtmlParser::parse(const QString& html)
{
    // Return a virtual root element containing the document
    // We pass "root" as tag, and the full html as inner.
    // This allows .select() to run on the full document.
    return new HtmlElement("root", "", html);
}

// Logic to find elements matching a basic selector
// Selector support: "tag", ".class", "tag.class", "a[href*='foo']"
QList<HtmlElement*> HtmlParser::parse(const QString& html, const QString& selector)
{
    QList<HtmlElement*> results;

    // 1. Parse selector
    QString tag;
    QString className;
    QString attrName;
    QString attrOp;
    QString attrVal;

    QString sel = selector;

    // Check for attribute selector [attr*=val]
    QRegularExpression attrRegex("\\[([a-zA-Z0-9_-]+)([*^$]?=)['\"]?([^'\"]*)['\"]?\\]");
    QRegularExpressionMatch attrMatch = attrRegex.match(sel);
    if (attrMatch.hasMatch()) {
        attrName = attrMatch.captured(1);
        attrOp = attrMatch.captured(2);
        attrVal = attrMatch.captured(3);
        sel.remove(attrMatch.capturedStart(), attrMatch.capturedLength());
    }

    // Check for class
    int dotIdx = sel.indexOf('.');
    if (dotIdx != -1) {
        className = sel.mid(dotIdx + 1);
        tag = sel.left(dotIdx);
        if (tag.isEmpty()) tag = ""; // universal
    } else {
        tag = sel;
    }

    if (tag.isEmpty() && className.isEmpty() && attrName.isEmpty()) return results;

    // 2. Scan HTML
    // We iterate through the string seeking matches
    // This is a naive scanner that handles nested same-tags.

    int pos = 0;
    // Regex for start tag of any kind: <([a-zA-Z0-9]+)([^>]*)>
    QRegularExpression tagStartRegex("<([a-zA-Z0-9]+)([^>]*)>");

    while (pos < html.length()) {
        QRegularExpressionMatch match = tagStartRegex.match(html, pos);
        if (!match.hasMatch()) break;

        QString foundTag = match.captured(1);
        QString foundAttrs = match.captured(2);
        int startPos = match.capturedStart();
        int endTagPos = match.capturedEnd();

        bool isSelfClosing = foundAttrs.trimmed().endsWith("/");
        bool isVoid = (foundTag.compare("img", Qt::CaseInsensitive) == 0 ||
                       foundTag.compare("br", Qt::CaseInsensitive) == 0 ||
                       foundTag.compare("hr", Qt::CaseInsensitive) == 0 ||
                       foundTag.compare("input", Qt::CaseInsensitive) == 0 ||
                       foundTag.compare("meta", Qt::CaseInsensitive) == 0);

        // Check if this tag matches selector
        bool matches = true;

        // Tag check
        if (!tag.isEmpty() && foundTag.compare(tag, Qt::CaseInsensitive) != 0) matches = false;

        // Class check
        if (matches && !className.isEmpty()) {
            QRegularExpression classRegex(QString("class\\s*=\\s*[\"']?([^\"'>]*)[\"']?"), QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch cm = classRegex.match(foundAttrs);
            QString classes = cm.captured(1);
            QStringList classList = classes.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (!classList.contains(className)) matches = false;
        }

        // Attr check
        if (matches && !attrName.isEmpty()) {
             QRegularExpression aRegex(QString("%1\\s*=\\s*[\"']?([^\"'>]*)[\"']?").arg(QRegularExpression::escape(attrName)), QRegularExpression::CaseInsensitiveOption);
             QRegularExpressionMatch am = aRegex.match(foundAttrs);
             if (!am.hasMatch()) {
                 matches = false;
             } else {
                 QString val = am.captured(1);
                 if (attrOp == "=" && val != attrVal) matches = false;
                 else if (attrOp == "*=" && !val.contains(attrVal)) matches = false;
                 else if (attrOp == "^=" && !val.startsWith(attrVal)) matches = false;
                 else if (attrOp == "$=" && !val.endsWith(attrVal)) matches = false;
             }
        }

        // Extract Inner HTML
        QString innerHtml;
        int nextSearchPos = endTagPos;

        if (isSelfClosing || isVoid) {
            innerHtml = "";
            nextSearchPos = endTagPos;
        } else {
            // Find closing tag, accounting for nesting
            int depth = 1;
            int current = endTagPos;

            // Regex to find next start OR end tag of SAME type
            // <foundTag ...> or </foundTag>
            QRegularExpression tagSearch(QString("<(/?)%1\\b[^>]*>").arg(QRegularExpression::escape(foundTag)), QRegularExpression::CaseInsensitiveOption);

            while (depth > 0) {
                QRegularExpressionMatch m = tagSearch.match(html, current);
                if (!m.hasMatch()) {
                    // Malformed HTML, just take remaining? Or stop?
                    // Let's assume unclosed = rest of string
                    current = html.length();
                    break;
                }

                if (m.captured(1) == "/") {
                    depth--;
                } else {
                    // Check if self-closing
                    if (!m.captured(0).endsWith("/>")) {
                        depth++;
                    }
                }

                if (depth == 0) {
                    innerHtml = html.mid(endTagPos, m.capturedStart() - endTagPos);
                    nextSearchPos = m.capturedEnd();
                } else {
                     current = m.capturedEnd();
                }
            }
        }

        if (matches) {
            results.append(new HtmlElement(foundTag, foundAttrs, innerHtml));
        }

        if (matches && !selector.contains(" ")) {
            // If simple selector, we found this block. We continue searching AFTER this block?
            // Actually `select` searches *descendants* too if using a recursive parser.
            // But here we are scanning linear.
            // If we found a match, we can continue searching inside it?
            // Our current logic scans linearly. If we found a match, we add it.
            // We should ideally search INSIDE the matched element too if we want recursive find for generic selectors?
            // But standard querySelectorAll usually returns elements in document order.

            // If we continue at `nextSearchPos`, we skip nested matches inside the one we just found.
            // e.g. div.foo > div.foo
            // If we match outer, we skip inner.
            // To find ALL, we should probably advance just by 1 char or past the start tag?
            // Advancing past start tag allows matching nested items.
            pos = endTagPos;
        } else {
            // No match, advance past start tag
            pos = endTagPos;
        }
    }

    return results;
}
