#include "JavascriptSource.h"
#include "network/NetworkAccessManager.h" // Include our NetworkAccessManager
#include "model/Manga.h" // For Manga class
#include "model/Chapter.h" // For Chapter class
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QJSValueIterator>
#include "HtmlParser.h"
#include "ExtensionManager.h"


// Helper class for console logging
class ConsoleWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleWrapper(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void log(const QString& msg) {
        qDebug() << "JS Console:" << msg;
    }
    // Overload for multiple arguments if needed, or handle simpler
    // Using QJSValue for flexible arguments
    Q_INVOKABLE void log(const QJSValue& val) {
         qDebug() << "JS Console:" << val.toString();
    }
};

#include "JavascriptSource.moc" // Needed if we define QObject in cpp

JavascriptSource::JavascriptSource(const QString& scriptPath, QJSEngine* engine, NetworkAccessManager* networkManager, QObject *parent)
    : SourceBase(parent)
    , m_scriptPath(scriptPath)
    , m_engine(engine)
{
    // Expose the network manager to the JavaScript engine
    QJSValue networkAccessManagerWrapper = m_engine->newQObject(networkManager);
    m_engine->globalObject().setProperty("Network", networkAccessManagerWrapper);

    // Expose the HtmlParser
    HtmlParser *htmlParser = new HtmlParser(this);
    QJSValue htmlWrapper = m_engine->newQObject(htmlParser);
    m_engine->globalObject().setProperty("Html", htmlWrapper);

    // Expose the console.log
    ConsoleWrapper *consoleWrapper = new ConsoleWrapper(this);
    QJSValue consoleObj = m_engine->newQObject(consoleWrapper);
    m_engine->globalObject().setProperty("console", consoleObj);

    initScript();

    // After script is initialized, we have the pkg name (from script object if available)
    // and can set the network manager's trust status and base URL.
    QString pkgName = m_scriptObject.property("pkg").toString();
    if (pkgName.isEmpty()) {
        // Fallback to filename if pkg is not defined in script
        pkgName = QFileInfo(m_scriptPath).baseName();
    }

    bool isTrusted = ExtensionManager::instance().isTrusted(pkgName);
    networkManager->setTrusted(isTrusted);
    networkManager->setAllowedBaseUrl(this->baseUrl());

}

JavascriptSource::~JavascriptSource()
{
    // QJSEngine and NetworkAccessManager are not owned by JavascriptSource
}

void JavascriptSource::initScript()
{
    QFile scriptFile(m_scriptPath);
    if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) { // flawfinder: ignore
        qWarning() << "Could not open JavaScript source file:" << m_scriptPath;
        return;
    }

    QTextStream stream(&scriptFile);
    QString scriptContents = stream.readAll();
    scriptFile.close();

    QJSValue result = m_engine->evaluate(scriptContents, m_scriptPath);

    if (result.isError()) {
        qCritical() << "Uncaught exception at" << result.property("lineNumber").toInt()
                    << ":" << result.toString();
        return;
    }

    // Assume the script defines a global object that represents the source
    // e.g., 'var MySource = { name: "...", id: ..., getPopularManga: function() { ... } };'
    // And that object is assigned to a global variable, or we simply use global functions.
    // For now, let's assume global functions/properties for simplicity and then evolve.

    QJSValue globalObject = m_engine->globalObject();
    if (globalObject.hasProperty("source")) {
        m_scriptObject = globalObject.property("source");
        if (!m_scriptObject.isObject()) {
            qWarning() << "JavaScript source script did not define a 'source' object properly.";
            m_scriptObject = QJSValue(); // Invalidate script object
        }
    } else {
        qWarning() << "JavaScript source script did not define a global 'source' object. Using global scope directly.";
        m_scriptObject = globalObject; // Use global object directly if no 'source' object is found
    }
}

QJSValue JavascriptSource::callJsFunction(const QString& functionName, const QJSValueList& args)
{
    if (m_scriptObject.isUndefined() || m_scriptObject.isNull()) {
        qCritical() << "JavaScript source object not initialized or invalid.";
        return m_engine->newErrorObject(QJSValue::GenericError, "JavaScript source object not initialized or invalid.");
    }

    QJSValue function = m_scriptObject.property(functionName);
    if (!function.isCallable()) {
        qCritical() << "JavaScript function '" << functionName << "' not found or not callable in script.";
        return m_engine->newErrorObject(QJSValue::GenericError, "JavaScript function not found or not callable.");
    }

    QJSValue result = function.callWithInstance(m_scriptObject, args);
    if (result.isError()) {
        qCritical() << "JavaScript function '" << functionName << "' failed:" << result.toString()
                    << "at line" << result.property("lineNumber").toInt();
    }
    return result;
}

QString JavascriptSource::name() const
{
    QJSValue value = m_scriptObject.property("name");
    return value.toString();
}

long JavascriptSource::id() const
{
    QJSValue value = m_scriptObject.property("id");
    return static_cast<long>(value.toNumber());
}

QString JavascriptSource::lang() const
{
    QJSValue value = m_scriptObject.property("lang");
    return value.toString();
}



QString JavascriptSource::baseUrl() const
{
    QJSValue value = m_scriptObject.property("baseUrl");
    return value.toString();
}

bool JavascriptSource::supportsLatest() const
{
    QJSValue value = m_scriptObject.property("supportsLatest");
    return value.toBool();
}

QList<Manga> JavascriptSource::getPopularManga()
{
    QList<Manga> mangaList;
    QJSValue jsResult = callJsFunction("getPopularManga");
    if (jsResult.isArray()) {
        QJSValueIterator it(jsResult);
        while (it.hasNext()) {
            it.next();
            QJSValue jsManga = it.value();
            if (jsManga.isObject()) {
                // Convert JS object to C++ Manga object
                // This is a simplified conversion, actual conversion might be more complex
                Manga manga(
                    -1, // ID, will be assigned by DB
                    this->id(), // source ID
                    jsManga.property("url").toString(),
                    jsManga.property("title").toString(),
                    jsManga.property("artist").toString(),
                    jsManga.property("author").toString(),
                    jsManga.property("description").toString(),
                    jsManga.property("genre").toString(),
                    static_cast<MangaStatus>(jsManga.property("status").toInt()),
                    jsManga.property("thumbnail_url").toString(),
                    jsManga.property("favorite").toBool(),
                    static_cast<long>(jsManga.property("lastUpdate").toNumber()),
                    static_cast<long>(jsManga.property("nextUpdate").toNumber()),
                    jsManga.property("fetchInterval").toInt(),
                    static_cast<long>(jsManga.property("dateAdded").toNumber()),
                    jsManga.property("viewerFlags").toInt(),
                    jsManga.property("chapterFlags").toInt(),
                    static_cast<long>(jsManga.property("coverLastModified").toNumber()),
                    static_cast<UpdateStrategy>(jsManga.property("updateStrategy").toInt()),
                    jsManga.property("initialized").toBool(),
                    static_cast<long>(jsManga.property("lastModifiedAt").toNumber()),
                    static_cast<long>(jsManga.property("favoriteModifiedAt").toNumber()),
                    jsManga.property("version").toInt(),
                    jsManga.property("notes").toString()
                );
                mangaList.append(manga);
            }
        }
    }
    return mangaList;
}

QList<Manga> JavascriptSource::getLatestUpdates()
{
    // Similar conversion logic as getPopularManga
    QList<Manga> mangaList;
    QJSValue jsResult = callJsFunction("getLatestUpdates");
    if (jsResult.isArray()) {
        QJSValueIterator it(jsResult);
        while (it.hasNext()) {
            it.next();
            QJSValue jsManga = it.value();
            if (jsManga.isObject()) {
                Manga manga(
                    -1, this->id(),
                    jsManga.property("url").toString(), jsManga.property("title").toString(),
                    jsManga.property("artist").toString(), jsManga.property("author").toString(),
                    jsManga.property("description").toString(), jsManga.property("genre").toString(),
                    static_cast<MangaStatus>(jsManga.property("status").toInt()),
                    jsManga.property("thumbnail_url").toString(), jsManga.property("favorite").toBool(),
                    static_cast<long>(jsManga.property("lastUpdate").toNumber()), static_cast<long>(jsManga.property("nextUpdate").toNumber()),
                    jsManga.property("fetchInterval").toInt(), static_cast<long>(jsManga.property("dateAdded").toNumber()),
                    jsManga.property("viewerFlags").toInt(), jsManga.property("chapterFlags").toInt(),
                    static_cast<long>(jsManga.property("coverLastModified").toNumber()),
                    static_cast<UpdateStrategy>(jsManga.property("updateStrategy").toInt()),
                    jsManga.property("initialized").toBool(),
                    static_cast<long>(jsManga.property("lastModifiedAt").toNumber()), static_cast<long>(jsManga.property("favoriteModifiedAt").toNumber()),
                    jsManga.property("version").toInt(), jsManga.property("notes").toString()
                );
                mangaList.append(manga);
            }
        }
    }
    return mangaList;
}

QList<Manga> JavascriptSource::getSearchManga(const QString& query)
{
    QList<Manga> mangaList;
    QJSValue jsResult = callJsFunction("getSearchManga", QJSValueList() << query);
    if (jsResult.isArray()) {
        QJSValueIterator it(jsResult);
        while (it.hasNext()) {
            it.next();
            QJSValue jsManga = it.value();
            if (jsManga.isObject()) {
                Manga manga(
                    -1, this->id(),
                    jsManga.property("url").toString(), jsManga.property("title").toString(),
                    jsManga.property("artist").toString(), jsManga.property("author").toString(),
                    jsManga.property("description").toString(), jsManga.property("genre").toString(),
                    static_cast<MangaStatus>(jsManga.property("status").toInt()),
                    jsManga.property("thumbnail_url").toString(), jsManga.property("favorite").toBool(),
                    static_cast<long>(jsManga.property("lastUpdate").toNumber()), static_cast<long>(jsManga.property("nextUpdate").toNumber()),
                    jsManga.property("fetchInterval").toInt(), static_cast<long>(jsManga.property("dateAdded").toNumber()),
                    jsManga.property("viewerFlags").toInt(), jsManga.property("chapterFlags").toInt(),
                    static_cast<long>(jsManga.property("coverLastModified").toNumber()),
                    static_cast<UpdateStrategy>(jsManga.property("updateStrategy").toInt()),
                    jsManga.property("initialized").toBool(),
                    static_cast<long>(jsManga.property("lastModifiedAt").toNumber()), static_cast<long>(jsManga.property("favoriteModifiedAt").toNumber()),
                    jsManga.property("version").toInt(), jsManga.property("notes").toString()
                );
                mangaList.append(manga);
            }
        }
    }
    return mangaList;
}

Manga JavascriptSource::getMangaDetails(const Manga& manga)
{
    // Need to pass Manga object to JS. QJSValue can wrap QObjects, but Manga is not a QObject.
    // Will need a custom conversion or pass relevant properties. For simplicity, passing ID and URL.
    QJSValueList args;
    args << QJSValue(static_cast<double>(manga.id())) << QJSValue(manga.url());

    QJSValue jsResult = callJsFunction("getMangaDetails", args);
    if (jsResult.isObject()) {
        // Create a new Manga object with updated details from JS
        Manga updatedManga(manga); // Start with existing manga details
        updatedManga.setArtist(jsResult.property("artist").toString());
        updatedManga.setAuthor(jsResult.property("author").toString());
        updatedManga.setDescription(jsResult.property("description").toString());
        updatedManga.setGenre(jsResult.property("genre").toString());
        updatedManga.setStatus(static_cast<MangaStatus>(jsResult.property("status").toInt()));
        updatedManga.setThumbnailUrl(jsResult.property("thumbnail_url").toString());
        updatedManga.setInitialized(jsResult.property("initialized").toBool());
        // ... update other properties as needed
        return updatedManga;
    }
    return manga; // Return original manga if JS call fails
}

QList<SChapter> JavascriptSource::getChapterList(const Manga& manga)
{
    QList<SChapter> chapterList;
    // Need to pass Manga object to JS. For simplicity, passing ID and URL.
    QJSValueList args;
    args << QJSValue(static_cast<double>(manga.id())) << QJSValue(manga.url());

    QJSValue jsResult = callJsFunction("getChapterList", args);
    if (jsResult.isArray()) {
        QJSValueIterator it(jsResult);
        while (it.hasNext()) {
            it.next();
            QJSValue jsChapter = it.value();
            if (jsChapter.isObject()) {
                SChapter chapter(
                    jsChapter.property("url").toString(),
                    jsChapter.property("name").toString(),
                    static_cast<long>(jsChapter.property("dateUpload").toNumber()),
                    static_cast<float>(jsChapter.property("chapterNumber").toNumber()),
                    jsChapter.property("scanlator").toString()
                );
                chapterList.append(chapter);
            }
        }
    }
    return chapterList;
}

QList<QString> JavascriptSource::getPageList(const Chapter& chapter)
{
    QList<QString> pageList;
    // Pass chapter details to JS
    QJSValueList args;
    args << QJSValue(static_cast<double>(chapter.id())) << QJSValue(chapter.url());

    QJSValue jsResult = callJsFunction("getPageList", args);
    if (jsResult.isArray()) {
        QJSValueIterator it(jsResult);
        while (it.hasNext()) {
            it.next();
            QJSValue jsPage = it.value();
            if (jsPage.isObject()) {
                // Expecting { index: 0, url: "..." }
                // We just need the URL for now
                pageList.append(jsPage.property("url").toString());
            } else if (jsPage.isString()) {
                pageList.append(jsPage.toString());
            }
        }
    }
    return pageList;
}
