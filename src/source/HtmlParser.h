#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>
#include <QRegularExpression>

class HtmlElement : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QString innerHtml READ innerHtml)
    Q_PROPERTY(QString outerHtml READ outerHtml)
    Q_PROPERTY(QString tagName READ tagName)

public:
    explicit HtmlElement(const QString& tag, const QString& attrStr, const QString& inner, QObject *parent = nullptr);

    QString text() const;
    QString innerHtml() const { return m_innerHtml; }
    QString outerHtml() const;
    QString tagName() const { return m_tagName; }

    Q_INVOKABLE QString attr(const QString& name);
    Q_INVOKABLE QVariantList select(const QString& selector); // Returns list of HtmlElement*
    Q_INVOKABLE QObject* first(const QString& selector);

private:
    QString m_tagName;
    QString m_attributes;
    QString m_innerHtml;

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

    // Helper to generic parse
    static QList<HtmlElement*> parse(const QString& html, const QString& selector);
};

#endif // HTMLPARSER_H
