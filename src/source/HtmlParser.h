#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>
#include <memory>
#include <gumbo.h>

struct GumboOutputDeleter {
    void operator()(GumboOutput* p) const {
        if (p) {
            gumbo_destroy_output(&kGumboDefaultOptions, p);
        }
    }
};

class HtmlElement : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QString innerHtml READ innerHtml)
    Q_PROPERTY(QString outerHtml READ outerHtml)
    Q_PROPERTY(QString tagName READ tagName)

public:
    explicit HtmlElement(std::shared_ptr<GumboOutput> tree, const GumboNode* node, QObject *parent = nullptr);

    Q_INVOKABLE QString text() const;
    Q_INVOKABLE QString innerHtml() const;
    Q_INVOKABLE QString outerHtml() const;
    Q_INVOKABLE QString tagName() const;

    Q_INVOKABLE QString attr(const QString& name) const;
    Q_INVOKABLE bool hasAttr(const QString& name) const;
    Q_INVOKABLE QVariantList select(const QString& selector);
    Q_INVOKABLE QObject* first(const QString& selector);

    const GumboNode* node() const { return m_node; }
    std::shared_ptr<GumboOutput> tree() const { return m_tree; }

private:
    std::shared_ptr<GumboOutput> m_tree;
    const GumboNode* m_node;

    friend class HtmlParser;
};

class HtmlParser : public QObject
{
    Q_OBJECT
public:
    explicit HtmlParser(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList select(const QString& html, const QString& selector);
    Q_INVOKABLE QObject* first(const QString& html, const QString& selector);
    Q_INVOKABLE QObject* parse(const QString& html);

    static QList<HtmlElement*> query(std::shared_ptr<GumboOutput> tree, const GumboNode* root, const QString& selector);
};

#endif // HTMLPARSER_H
