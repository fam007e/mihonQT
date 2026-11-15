#ifndef MYANIMELISTTRACKER_H
#define MYANIMELISTTRACKER_H

#include "TrackingService.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

class MyAnimeListTracker : public TrackingService
{
    Q_OBJECT

public:
    static MyAnimeListTracker& instance();

    QString name() const override { return "MyAnimeList"; }
    QString id() const override { return "myanimelist"; }
    QUrl authUrl() const override;

    bool isLoggedIn() const override;
    void login(const QString& authCode) override;
    void logout() override;

    void search(const QString& query) override;
    void bind(long localMangaId, long remoteId) override;
    void unbind(long localMangaId) override;

    void update(long localMangaId, int chaptersRead, int status, float score) override;
    TrackingEntry getEntry(long localMangaId) override;

    QString clientId() const;
    QString redirectUri() const;

private:
    explicit MyAnimeListTracker(QObject *parent = nullptr);

    QString mapStatus(int mihonStatus);

    QNetworkAccessManager *m_networkManager;
    QString m_accessToken;
    QString m_refreshToken;
    QString m_codeVerifier;

    QMap<long, long> m_bindings;
};

#endif // MYANIMELISTTRACKER_H
