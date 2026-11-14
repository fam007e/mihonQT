#ifndef JAVASCRIPTSOURCE_H
#define JAVASCRIPTSOURCE_H

#include "SourceBase.h"
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QList>

class NetworkAccessManager;

class JavascriptSource : public SourceBase
{
    Q_OBJECT
public:
    explicit JavascriptSource(const QString& scriptPath, QJSEngine* engine = nullptr, NetworkAccessManager* networkManager = nullptr, QObject *parent = nullptr);
    ~JavascriptSource() override;

    // SourceBase overrides
    QString name() const override;
    long id() const override;
    QString lang() const override;
    QString baseUrl() const override;
    bool supportsLatest() const override;

    QList<Manga> getPopularManga(int page) override;
    QList<Manga> getLatestUpdates(int page) override;
    QList<Manga> getSearchManga(const QString& query, int page) override;
    Manga getMangaDetails(const Manga& manga) override;
    QList<SChapter> getChapterList(const Manga& manga) override;
    QList<QString> getPageList(const Chapter& chapter) override;

private:
    QString m_scriptPath;
    QString m_scriptContent;
    QString m_name;
    long m_id = 0;
    QString m_lang;
    QString m_baseUrl;
    bool m_supportsLatest = false;
    QString m_pkgName;

    QJSValue callJsMethod(QJSEngine& engine, const QString& functionName, const QJSValueList& args = QJSValueList());
};

#endif // JAVASCRIPTSOURCE_H

