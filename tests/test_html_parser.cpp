#include <QTest>
#include <QObject>
#include "../src/source/HtmlParser.h"

class TestHtmlParser : public QObject
{
    Q_OBJECT

private slots:
    void testBasicParseAndTagSelector();
    void testClassAndIdSelectors();
    void testAttributeSelectors();
    void testCombinatorSelectors();
    void testTextAndAttrExtraction();
    void testEmptyAndMalformedHtml();
    void testDeeplyNestedHtml();
    void testBufferLifetimeNoUseAfterFree();
};

void TestHtmlParser::testBasicParseAndTagSelector()
{
    HtmlParser parser;
    QString html = "<html><body><h1>Title</h1><p class='intro'>First paragraph</p><p>Second paragraph</p></body></html>";
    
    QObject* parsedDoc = parser.parse(html);
    QVERIFY(parsedDoc != nullptr);
    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    QVERIFY(doc != nullptr);

    QVariantList paragraphs = doc->select("p");
    QCOMPARE(paragraphs.size(), 2);

    QObject* h1Obj = doc->first("h1");
    QVERIFY(h1Obj != nullptr);
    HtmlElement* h1 = qobject_cast<HtmlElement*>(h1Obj);
    QVERIFY(h1 != nullptr);
    QCOMPARE(h1->text().trimmed(), QString("Title"));

    delete parsedDoc;
}

void TestHtmlParser::testClassAndIdSelectors()
{
    HtmlParser parser;
    QString html = "<div id='main' class='container'><span class='highlight'>Text 1</span><span class='normal'>Text 2</span></div>";

    QObject* parsedDoc = parser.parse(html);
    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    QVERIFY(doc != nullptr);

    QVariantList idMatches = doc->select("#main");
    QCOMPARE(idMatches.size(), 1);

    QVariantList classMatches = doc->select(".highlight");
    QCOMPARE(classMatches.size(), 1);
    HtmlElement* el = idMatches[0].value<HtmlElement*>();
    QVERIFY(el != nullptr);
    QCOMPARE(el->attr("id"), QString("main"));
    QCOMPARE(el->attr("class"), QString("container"));

    delete parsedDoc;
}

void TestHtmlParser::testAttributeSelectors()
{
    HtmlParser parser;
    QString html = "<section x-data='{ open: false }'>"
                   "  <img src='https://example.com/1.png' alt='Page 1' />"
                   "  <img src='https://example.com/2.png' alt='Page 2' />"
                   "  <a href='/read/ch1'>Link</a>"
                   "</section>";

    QObject* parsedDoc = parser.parse(html);
    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    QVERIFY(doc != nullptr);

    QVariantList xdata = doc->select("section[x-data]");
    QCOMPARE(xdata.size(), 1);

    QVariantList imgs = doc->select("img[src]");
    QCOMPARE(imgs.size(), 2);

    QVariantList links = doc->select("a[href='/read/ch1']");
    QCOMPARE(links.size(), 1);

    delete parsedDoc;
}

void TestHtmlParser::testCombinatorSelectors()
{
    HtmlParser parser;
    QString html = "<div class='wrapper'>"
                   "  <article>"
                   "    <a href='/a1'><span>A1</span></a>"
                   "  </article>"
                   "  <a href='/a2'>A2</a>"
                   "</div>";

    QObject* parsedDoc = parser.parse(html);
    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    QVERIFY(doc != nullptr);

    // Child combinator
    QVariantList directLinks = doc->select(".wrapper > a");
    QCOMPARE(directLinks.size(), 1);

    // Descendant combinator
    QVariantList allLinks = doc->select(".wrapper a");
    QCOMPARE(allLinks.size(), 2);

    delete parsedDoc;
}

void TestHtmlParser::testTextAndAttrExtraction()
{
    HtmlParser parser;
    QString html = "<div class='card' data-id='42'>  Hello <b>World</b>!  </div>";

    QObject* parsedDoc = parser.parse(html);
    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    QVERIFY(doc != nullptr);

    QObject* cardObj = doc->first(".card");
    QVERIFY(cardObj != nullptr);
    HtmlElement* card = qobject_cast<HtmlElement*>(cardObj);
    QVERIFY(card != nullptr);

    QCOMPARE(card->text().simplified(), QString("Hello World!"));
    QCOMPARE(card->attr("data-id"), QString("42"));
    QCOMPARE(card->attr("nonexistent"), QString(""));
    QVERIFY(card->hasAttr("data-id"));
    QVERIFY(!card->hasAttr("nonexistent"));

    delete parsedDoc;
}

void TestHtmlParser::testEmptyAndMalformedHtml()
{
    HtmlParser parser;

    // Empty HTML
    QObject* emptyDoc = parser.parse("");
    QVERIFY(emptyDoc != nullptr);
    HtmlElement* elEmpty = qobject_cast<HtmlElement*>(emptyDoc);
    QVERIFY(elEmpty != nullptr);
    QCOMPARE(elEmpty->select("p").size(), 0);
    delete emptyDoc;

    // Malformed HTML (unclosed tags, broken attributes, null bytes)
    QString broken = "<div class='unclosed' <span><img src=foo> broken text >>> <<<";
    QObject* brokenDoc = parser.parse(broken);
    QVERIFY(brokenDoc != nullptr);
    HtmlElement* elBroken = qobject_cast<HtmlElement*>(brokenDoc);
    QVERIFY(elBroken != nullptr);
    QVERIFY(elBroken->select("img").size() >= 1);
    delete brokenDoc;
}

void TestHtmlParser::testDeeplyNestedHtml()
{
    HtmlParser parser;
    QString nested;
    for (int i = 0; i < 100; ++i) {
        nested += "<div>";
    }
    nested += "<span id='deep'>Target</span>";
    for (int i = 0; i < 100; ++i) {
        nested += "</div>";
    }

    QObject* parsedDoc = parser.parse(nested);
    QVERIFY(parsedDoc != nullptr);
    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    QVERIFY(doc != nullptr);

    QObject* deepObj = doc->first("#deep");
    QVERIFY(deepObj != nullptr);
    HtmlElement* deep = qobject_cast<HtmlElement*>(deepObj);
    QVERIFY(deep != nullptr);
    QCOMPARE(deep->text(), QString("Target"));

    delete parsedDoc;
}

void TestHtmlParser::testBufferLifetimeNoUseAfterFree()
{
    // Ensure that even if parser temporary is destroyed or many elements are extracted,
    // GumboDocumentState shared_ptr retains buffer validity.
    HtmlElement* retainedElement = nullptr;
    {
        HtmlParser parser;
        QString html = "<div><p id='target'>Safe Text</p></div>";
        QObject* parsedDoc = parser.parse(html);
        HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
        QObject* pObj = doc->first("#target");
        retainedElement = qobject_cast<HtmlElement*>(pObj);
        QVERIFY(retainedElement != nullptr);
        retainedElement->setParent(nullptr); // Detach from parsedDoc to test standalone lifetime
        // Delete parsedDoc; retainedElement should still hold shared_ptr to doc state
        delete parsedDoc;
    }

    // Now access retained element: buffer should be valid, no crash or use-after-free
    QCOMPARE(retainedElement->text(), QString("Safe Text"));
    QCOMPARE(retainedElement->attr("id"), QString("target"));
    delete retainedElement;
}

QTEST_MAIN(TestHtmlParser)
#include "test_html_parser.moc"
