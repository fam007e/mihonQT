#include "JavascriptSource.h"
#include "network/NetworkAccessManager.h"
#include "model/Manga.h"
#include "model/Chapter.h"
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
    Q_INVOKABLE void log(const QJSValue& val) {
        qDebug() << "JS Console:" << val.toString();
    }
};

#include "JavascriptSource.moc"

// ---- Helpers to extract all data from QJSValue while engine is alive ----

static QString safeStr(const QJSValue& obj, const char* key) {
    QJSValue v = obj.property(key);
    return v.isUndefined() ? QString() : v.toString();
}

static long safeLong(const QJSValue& obj, const char* key) {
    QJSValue v = obj.property(key);
    return v.isUndefined() ? 0L : static_cast<long>(v.toNumber());
}

static int safeInt(const QJSValue& obj, const char* key) {
    QJSValue v = obj.property(key);
    return v.isUndefined() ? 0 : v.toInt();
}

static bool safeBool(const QJSValue& obj, const char* key) {
    QJSValue v = obj.property(key);
    return v.isUndefined() ? false : v.toBool();
}

static float safeFloat(const QJSValue& obj, const char* key) {
    QJSValue v = obj.property(key);
    return v.isUndefined() ? 0.0f : static_cast<float>(v.toNumber());
}

// Extract a manga list from a QJSValue array, fully copying all strings/numbers
// out of the JS heap before the engine can be destroyed.
static QList<Manga> extractMangaList(const QJSValue& jsResult, long sourceId)
{
    QList<Manga> mangaList;
    if (!jsResult.isArray()) return mangaList;

    int length = jsResult.property("length").toInt();
    for (int i = 0; i < length; ++i) {
        QJSValue jsManga = jsResult.property(i);
        if (!jsManga.isObject()) continue;

        Manga manga(
            -1,
            sourceId,
            safeStr(jsManga, "url"),
            safeStr(jsManga, "title"),
            safeStr(jsManga, "artist"),
            safeStr(jsManga, "author"),
            safeStr(jsManga, "description"),
            safeStr(jsManga, "genre"),
            static_cast<MangaStatus>(safeInt(jsManga, "status")),
            safeStr(jsManga, "thumbnail_url"),
            safeBool(jsManga, "favorite"),
            safeLong(jsManga, "lastUpdate"),
            safeLong(jsManga, "nextUpdate"),
            safeInt(jsManga, "fetchInterval"),
            safeLong(jsManga, "dateAdded"),
            safeInt(jsManga, "viewerFlags"),
            safeInt(jsManga, "chapterFlags"),
            safeLong(jsManga, "coverLastModified"),
            static_cast<UpdateStrategy>(safeInt(jsManga, "updateStrategy")),
            safeBool(jsManga, "initialized"),
            safeLong(jsManga, "lastModifiedAt"),
            safeLong(jsManga, "favoriteModifiedAt"),
            safeInt(jsManga, "version"),
            safeStr(jsManga, "notes")
        );
        mangaList.append(manga);
    }
    return mangaList;
}

static QList<SChapter> extractChapterList(const QJSValue& jsResult)
{
    QList<SChapter> chapterList;
    if (!jsResult.isArray()) return chapterList;

    int length = jsResult.property("length").toInt();
    for (int i = 0; i < length; ++i) {
        QJSValue jsChapter = jsResult.property(i);
        if (!jsChapter.isObject()) continue;

        SChapter chapter(
            safeStr(jsChapter, "url"),
            safeStr(jsChapter, "name"),
            safeLong(jsChapter, "dateUpload"),
            safeFloat(jsChapter, "chapterNumber"),
            safeStr(jsChapter, "scanlator")
        );
        chapterList.append(chapter);
    }
    return chapterList;
}

static QList<QString> extractPageList(const QJSValue& jsResult)
{
    QList<QString> pageList;
    if (!jsResult.isArray()) return pageList;

    int length = jsResult.property("length").toInt();
    for (int i = 0; i < length; ++i) {
        QJSValue jsPage = jsResult.property(i);
        if (jsPage.isObject()) {
            pageList.append(safeStr(jsPage, "url"));
        } else if (jsPage.isString()) {
            pageList.append(jsPage.toString());
        }
    }
    return pageList;
}

// ---- JavascriptSource implementation ----

JavascriptSource::JavascriptSource(const QString& scriptPath, QJSEngine* /*engine*/, NetworkAccessManager* /*networkManager*/, QObject *parent)
    : SourceBase(parent)
    , m_scriptPath(scriptPath)
{
    QFile scriptFile(m_scriptPath);
    if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) { // flawfinder: ignore
        qWarning() << "Could not open JavaScript source file:" << m_scriptPath;
        return;
    }

    QTextStream stream(&scriptFile);
    m_scriptContent = stream.readAll();
    scriptFile.close();

    // Use a temporary engine just to extract static metadata (name, id, etc.)
    {
        QJSEngine initEngine;
        ConsoleWrapper consoleWrapper;
        initEngine.globalObject().setProperty("console", initEngine.newQObject(&consoleWrapper));

        // Provide a stub Html and Network so the script doesn't error on missing globals
        HtmlParser htmlParser;
        initEngine.globalObject().setProperty("Html", initEngine.newQObject(&htmlParser));

        QJSValue result = initEngine.evaluate(m_scriptContent, m_scriptPath);
        if (result.isError()) {
            qCritical() << "Error evaluating script" << m_scriptPath << ":" << result.toString();
            return;
        }

        QJSValue sourceObj = initEngine.globalObject().property("source");
        if (!sourceObj.isObject()) {
            sourceObj = initEngine.globalObject();
        }

        m_name = sourceObj.property("name").toString();
        m_id = static_cast<long>(sourceObj.property("id").toNumber());
        m_lang = sourceObj.property("lang").toString();
        m_baseUrl = sourceObj.property("baseUrl").toString();
        m_supportsLatest = sourceObj.property("supportsLatest").toBool();
        m_pkgName = sourceObj.property("pkg").toString();
        if (m_pkgName.isEmpty()) {
            m_pkgName = QFileInfo(m_scriptPath).baseName();
        }
        // initEngine destructs here safely — no JS objects leaked
    }
}

JavascriptSource::~JavascriptSource()
{
}

QString JavascriptSource::name() const { return m_name; }
long JavascriptSource::id() const { return m_id; }
QString JavascriptSource::lang() const { return m_lang; }
QString JavascriptSource::baseUrl() const { return m_baseUrl; }
bool JavascriptSource::supportsLatest() const { return m_supportsLatest; }

QJSValue JavascriptSource::callJsMethod(QJSEngine& engine, const QString& functionName, const QJSValueList& args)
{
    // All helper objects are parented to the engine so they get destroyed with it
    ConsoleWrapper *consoleWrapper = new ConsoleWrapper(&engine);
    engine.globalObject().setProperty("console", engine.newQObject(consoleWrapper));

    HtmlParser *htmlParser = new HtmlParser(&engine);
    engine.globalObject().setProperty("Html", engine.newQObject(htmlParser));

    NetworkAccessManager *netAccess = new NetworkAccessManager(&engine, &engine);
    netAccess->setAllowedBaseUrl(m_baseUrl);
    netAccess->setTrusted(ExtensionManager::instance().isTrusted(m_pkgName));
    engine.globalObject().setProperty("Network", engine.newQObject(netAccess));

    QJSValue evalResult = engine.evaluate(m_scriptContent, m_scriptPath);
    if (evalResult.isError()) {
        qCritical() << "Uncaught exception evaluating" << m_scriptPath << ":"
                    << evalResult.toString() << "at line"
                    << evalResult.property("lineNumber").toInt();
        return QJSValue();
    }

    QJSValue sourceObj = engine.globalObject().property("source");
    if (!sourceObj.isObject()) {
        sourceObj = engine.globalObject();
    }

    QJSValue function = sourceObj.property(functionName);
    if (!function.isCallable()) {
        qCritical() << "JavaScript function '" << functionName << "' not found or not callable in script.";
        return QJSValue();
    }

    QJSValue result = function.callWithInstance(sourceObj, args);
    if (result.isError()) {
        qCritical() << "JavaScript function '" << functionName << "' failed:" << result.toString()
                    << "at line" << result.property("lineNumber").toInt();
    }
    return result;
}

QList<Manga> JavascriptSource::getPopularManga()
{
    QJSEngine engine;
    QJSValue jsResult = callJsMethod(engine, "getPopularManga");
    // Extract ALL data from JS values before engine destructs
    QList<Manga> list = extractMangaList(jsResult, this->id());
    // Clear JS references before engine teardown
    jsResult = QJSValue();
    engine.collectGarbage();
    return list;
}

QList<Manga> JavascriptSource::getLatestUpdates()
{
    QJSEngine engine;
    QJSValue jsResult = callJsMethod(engine, "getLatestUpdates");
    QList<Manga> list = extractMangaList(jsResult, this->id());
    jsResult = QJSValue();
    engine.collectGarbage();
    return list;
}

QList<Manga> JavascriptSource::getSearchManga(const QString& query)
{
    QJSEngine engine;
    QJSValueList args;
    args << QJSValue(query);
    QJSValue jsResult = callJsMethod(engine, "getSearchManga", args);
    QList<Manga> list = extractMangaList(jsResult, this->id());
    jsResult = QJSValue();
    engine.collectGarbage();
    return list;
}

Manga JavascriptSource::getMangaDetails(const Manga& manga)
{
    QJSEngine engine;
    QJSValueList args;
    args << QJSValue(static_cast<double>(manga.id())) << QJSValue(manga.url());

    QJSValue jsResult = callJsMethod(engine, "getMangaDetails", args);
    if (!jsResult.isObject()) return manga;

    // Extract all values out of JS heap immediately
    Manga updatedManga(manga);
    if (jsResult.hasProperty("artist")) updatedManga.setArtist(safeStr(jsResult, "artist"));
    if (jsResult.hasProperty("author")) updatedManga.setAuthor(safeStr(jsResult, "author"));
    if (jsResult.hasProperty("description")) updatedManga.setDescription(safeStr(jsResult, "description"));
    if (jsResult.hasProperty("genre")) updatedManga.setGenre(safeStr(jsResult, "genre"));
    if (jsResult.hasProperty("status")) updatedManga.setStatus(static_cast<MangaStatus>(safeInt(jsResult, "status")));
    if (jsResult.hasProperty("thumbnail_url")) updatedManga.setThumbnailUrl(safeStr(jsResult, "thumbnail_url"));
    if (jsResult.hasProperty("initialized")) updatedManga.setInitialized(safeBool(jsResult, "initialized"));

    jsResult = QJSValue();
    engine.collectGarbage();
    return updatedManga;
}

QList<SChapter> JavascriptSource::getChapterList(const Manga& manga)
{
    QJSEngine engine;
    QJSValueList args;
    args << QJSValue(static_cast<double>(manga.id())) << QJSValue(manga.url());

    QJSValue jsResult = callJsMethod(engine, "getChapterList", args);
    QList<SChapter> list = extractChapterList(jsResult);
    jsResult = QJSValue();
    engine.collectGarbage();
    return list;
}

QList<QString> JavascriptSource::getPageList(const Chapter& chapter)
{
    QJSEngine engine;
    QJSValueList args;
    args << QJSValue(static_cast<double>(chapter.id())) << QJSValue(chapter.url());

    QJSValue jsResult = callJsMethod(engine, "getPageList", args);
    QList<QString> list = extractPageList(jsResult);
    jsResult = QJSValue();
    engine.collectGarbage();
    return list;
}
