#ifndef MANGAUPDATESTRACKER_H
#define MANGAUPDATESTRACKER_H

#include "TrackingService.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

class MangaUpdatesTracker : public TrackingService
{
    Q_OBJECT

public:
    static MangaUpdatesTracker& instance();

    QString name() const override { return "MangaUpdates"; }
    QString id() const override { return "mangaupdates"; }
    QUrl authUrl() const override;

    bool isLoggedIn() const override;
    void login(const QString& authCode) override;
    void loginWithCredentials(const QString& username, const QString& password);
    void logout() override;

    void search(const QString& query) override;
    void bind(long localMangaId, long remoteId) override;
    void unbind(long localMangaId) override;

    void update(long localMangaId, int chaptersRead, int status, float score) override;
    TrackingEntry getEntry(long localMangaId) override;

private:
    explicit MangaUpdatesTracker(QObject *parent = nullptr);

    int mapStatus(int mihonStatus);

    QNetworkAccessManager *m_networkManager;
    QString m_sessionToken;
    QString m_username;

    QMap<long, long> m_bindings;
};

#endif // MANGAUPDATESTRACKER_H
