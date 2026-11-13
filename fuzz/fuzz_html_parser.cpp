#include <stdint.h>
#include <stddef.h>
#include <QString>
#include <QByteArray>
#include "../src/source/HtmlParser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0 || size > 1024 * 1024) { // Cap at 1MB to prevent excessive fuzz timeouts
        return 0;
    }

    // Convert fuzzer input to QString (HTML content)
    QByteArray raw(reinterpret_cast<const char*>(data), static_cast<int>(size));
    QString html = QString::fromUtf8(raw);

    HtmlParser parser;
    QObject* parsedDoc = parser.parse(html);
    if (!parsedDoc) {
        return 0;
    }

    HtmlElement* doc = qobject_cast<HtmlElement*>(parsedDoc);
    if (doc) {
        // Exercise text and attribute extractors
        QString text = doc->text();
        QString tagName = doc->tagName();
        QString outerHtml = doc->outerHtml();

        // Exercise various CSS selectors
        doc->select("div");
        doc->select(".chapter");
        doc->select("#content");
        doc->select("section[x-data] > img");
        doc->select("a[href]");
        doc->select("p.intro > span");
        doc->first("img");
        doc->first("a");
    }

    delete parsedDoc;
    return 0;
}
