#ifndef ANILISTTRACKER_H
#define ANILISTTRACKER_H

#include "TrackingService.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

class AniListTracker : public TrackingService
{
    Q_OBJECT

public:
    static AniListTracker& instance();

    QString name() const override { return "AniList"; }
    QString id() const override { return "anilist"; }
    QUrl authUrl() const override;

    bool isLoggedIn() const override;
    void login(const QString& authCode) override;
    void logout() override;

    void search(const QString& query) override;
    void bind(long localMangaId, long remoteId) override;
    void unbind(long localMangaId) override;

    void update(long localMangaId, int chaptersRead, int status, float score) override;
    TrackingEntry getEntry(long localMangaId) override;

private:
    explicit AniListTracker(QObject *parent = nullptr);

    void executeGraphQL(const QString& query, const QJsonObject& variables,
                        std::function<void(const QJsonObject&)> callback);
    int mapStatus(int mihonStatus);

    QNetworkAccessManager *m_networkManager;
    QString m_accessToken;
    long m_userId = 0;

    // Local binding storage: localMangaId -> remoteMediaId
    QMap<long, long> m_bindings;

    static const QString CLIENT_ID;
    static const QString REDIRECT_URI;
};

#endif // ANILISTTRACKER_H
