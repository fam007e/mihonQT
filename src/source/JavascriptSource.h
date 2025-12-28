#ifndef JAVASCRIPTSOURCE_H
#define JAVASCRIPTSOURCE_H

#include "SourceBase.h"
#include <QJSEngine> // Include QJSEngine
#include <QJSValue> // For QJSValue
#include "network/NetworkAccessManager.h" // Include our NetworkAccessManager

class QNetworkAccessManager; // Forward declaration (no longer needed here, but keeping for safety if other files still forward declare it)

class JavascriptSource : public SourceBase
{
    Q_OBJECT
public:
    explicit JavascriptSource(const QString& scriptPath, QJSEngine* engine, NetworkAccessManager* networkManager, QObject *parent = nullptr);
    ~JavascriptSource() override;

    // SourceBase overrides
    QString name() const override;
    long id() const override;
    QString lang() const override;
    QString baseUrl() const override;
    bool supportsLatest() const override;

    QList<Manga> getPopularManga() override;
    QList<Manga> getLatestUpdates() override;
    QList<Manga> getSearchManga(const QString& query) override;
    Manga getMangaDetails(const Manga& manga) override;
    QList<SChapter> getChapterList(const Manga& manga) override;
    QList<QString> getPageList(const Chapter& chapter) override;

private:
    QString m_scriptPath;
    QJSEngine* m_engine;
    QJSValue m_scriptObject; // Holds the JavaScript object representing the source

    void initScript();
    QJSValue callJsFunction(const QString& functionName, const QJSValueList& args = QJSValueList());
};

#endif // JAVASCRIPTSOURCE_H
