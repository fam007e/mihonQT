#ifndef TRACKINGSERVICE_H
#define TRACKINGSERVICE_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QJsonObject>

struct TrackingEntry {
    long id = 0;              // Tracker's ID for this entry
    long mangaId = 0;         // Local manga ID
    QString title;
    int status = 0;           // Reading, Completed, Dropped, etc.
    int chaptersRead = 0;
    int totalChapters = 0;
    float score = 0.0f;
    QString trackingUrl;
};

struct TrackingSearchResult {
    long remoteId = 0;
    QString title;
    QString coverUrl;
    QString synopsis;
    int totalChapters = 0;
};

class TrackingService : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Reading = 1,
        Completed = 2,
        OnHold = 3,
        Dropped = 4,
        PlanToRead = 5
    };
    Q_ENUM(Status)

    explicit TrackingService(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~TrackingService() = default;

    virtual QString name() const = 0;
    virtual QString id() const = 0;
    virtual QUrl authUrl() const = 0;

    virtual bool isLoggedIn() const = 0;
    virtual void login(const QString& authCode) = 0;
    virtual void logout() = 0;

    virtual void search(const QString& query) = 0;
    virtual void bind(long localMangaId, long remoteId) = 0;
    virtual void unbind(long localMangaId) = 0;

    virtual void update(long localMangaId, int chaptersRead, int status, float score) = 0;
    virtual TrackingEntry getEntry(long localMangaId) = 0;

signals:
    void loginSuccess();
    void loginFailed(const QString& error);
    void searchResults(const QList<TrackingSearchResult>& results);
    void bindSuccess(long localMangaId);
    void updateSuccess(long localMangaId);
    void error(const QString& message);
};

#endif // TRACKINGSERVICE_H
