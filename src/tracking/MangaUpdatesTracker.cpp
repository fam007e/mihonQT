#include "MangaUpdatesTracker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDebug>

MangaUpdatesTracker& MangaUpdatesTracker::instance()
{
    static MangaUpdatesTracker instance;
    return instance;
}

MangaUpdatesTracker::MangaUpdatesTracker(QObject *parent)
    : TrackingService(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    QSettings settings("MihonQT", "MihonQT");
    m_sessionToken = settings.value("tracking/mangaupdates_token", "").toString();
    m_username = settings.value("tracking/mangaupdates_username", "").toString();
}

QUrl MangaUpdatesTracker::authUrl() const
{
    return QUrl("https://www.mangaupdates.com/login.html");
}

bool MangaUpdatesTracker::isLoggedIn() const
{
    return !m_sessionToken.isEmpty();
}

void MangaUpdatesTracker::login(const QString& authCode)
{
    // If input format is "username:password", authenticate directly
    if (authCode.contains(':')) {
        int idx = authCode.indexOf(':');
        QString user = authCode.left(idx).trimmed();
        QString pass = authCode.mid(idx + 1).trimmed();
        loginWithCredentials(user, pass);
        return;
    }

    // Otherwise treat as a direct session token
    m_sessionToken = authCode.trimmed();
    QSettings settings("MihonQT", "MihonQT");
    settings.setValue("tracking/mangaupdates_token", m_sessionToken);
    emit loginSuccess();
}

void MangaUpdatesTracker::loginWithCredentials(const QString& username, const QString& password)
{
    QUrl url("https://api.mangaupdates.com/v1/account/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    QNetworkReply *reply = m_networkManager->put(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed(reply->errorString() + ": " + reply->readAll());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();

        if (root.contains("context")) {
            m_sessionToken = root["context"].toObject()["session_token"].toString();
            m_username = username;

            QSettings settings("MihonQT", "MihonQT");
            settings.setValue("tracking/mangaupdates_token", m_sessionToken);
            settings.setValue("tracking/mangaupdates_username", m_username);

            emit loginSuccess();
        } else {
            emit loginFailed("Invalid response from MangaUpdates: " + root["reason"].toString());
        }
    });
}

void MangaUpdatesTracker::logout()
{
    m_sessionToken.clear();
    m_username.clear();

    QSettings settings("MihonQT", "MihonQT");
    settings.remove("tracking/mangaupdates_token");
    settings.remove("tracking/mangaupdates_username");
}

void MangaUpdatesTracker::search(const QString& query)
{
    QUrl url("https://api.mangaupdates.com/v1/series/search");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["search"] = query;
    body["stype"] = "title";
    body["perpage"] = 10;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray resultsArray = doc.object()["results"].toArray();
        QList<TrackingSearchResult> results;

        for (const QJsonValue& val : resultsArray) {
            QJsonObject rec = val.toObject()["record"].toObject();
            TrackingSearchResult r;
            r.remoteId = rec["series_id"].toInteger();
            r.title = rec["title"].toString();
            r.synopsis = rec["description"].toString();
            r.totalChapters = 0;

            QJsonObject imgObj = rec["image"].toObject()["url"].toObject();
            r.coverUrl = imgObj["original"].toString();
            if (r.coverUrl.isEmpty()) {
                r.coverUrl = imgObj["thumb"].toString();
            }

            results.append(r);
        }

        emit searchResults(results);
    });
}

void MangaUpdatesTracker::bind(long localMangaId, long remoteId)
{
    m_bindings[localMangaId] = remoteId;

    QSettings settings("MihonQT", "MihonQT");
    settings.setValue(QString("tracking/mangaupdates_bind_%1").arg(localMangaId), static_cast<qlonglong>(remoteId));

    emit bindSuccess(localMangaId);
}

void MangaUpdatesTracker::unbind(long localMangaId)
{
    m_bindings.remove(localMangaId);

    QSettings settings("MihonQT", "MihonQT");
    settings.remove(QString("tracking/mangaupdates_bind_%1").arg(localMangaId));
    settings.remove(QString("tracking/mangaupdates_entry_chapters_%1").arg(localMangaId));
    settings.remove(QString("tracking/mangaupdates_entry_status_%1").arg(localMangaId));
    settings.remove(QString("tracking/mangaupdates_entry_score_%1").arg(localMangaId));
}

void MangaUpdatesTracker::update(long localMangaId, int chaptersRead, int status, float score)
{
    if (!m_bindings.contains(localMangaId)) {
        QSettings settings("MihonQT", "MihonQT");
        long savedId = settings.value(QString("tracking/mangaupdates_bind_%1").arg(localMangaId), 0).toLongLong();
        if (savedId > 0) {
            m_bindings[localMangaId] = savedId;
        } else {
            emit error("Manga not bound to MangaUpdates");
            return;
        }
    }

    if (m_sessionToken.isEmpty()) {
        emit error("Not logged in to MangaUpdates");
        return;
    }

    long remoteId = m_bindings[localMangaId];

    QUrl url("https://api.mangaupdates.com/v1/lists/series/update");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_sessionToken).toUtf8());

    QJsonObject seriesItem;
    QJsonObject seriesObj;
    seriesObj["id"] = static_cast<double>(remoteId);
    seriesItem["series"] = seriesObj;
    seriesItem["list_id"] = mapStatus(status);

    QJsonObject statusObj;
    statusObj["chapter"] = chaptersRead;
    seriesItem["status"] = statusObj;

    QJsonArray bodyArray;
    bodyArray.append(seriesItem);

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(bodyArray).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, localMangaId, remoteId, chaptersRead, status, score]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString() + ": " + reply->readAll());
            return;
        }

        QSettings settings("MihonQT", "MihonQT");
        settings.setValue(QString("tracking/mangaupdates_entry_chapters_%1").arg(localMangaId), chaptersRead);
        settings.setValue(QString("tracking/mangaupdates_entry_status_%1").arg(localMangaId), status);
        settings.setValue(QString("tracking/mangaupdates_entry_score_%1").arg(localMangaId), score);

        // Update score/rating if provided
        if (score > 0 && !m_sessionToken.isEmpty()) {
            QUrl ratingUrl(QString("https://api.mangaupdates.com/v1/series/%1/rating").arg(remoteId));
            QNetworkRequest ratingReq(ratingUrl);
            ratingReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            ratingReq.setRawHeader("Authorization", QString("Bearer %1").arg(m_sessionToken).toUtf8());

            QJsonObject rBody;
            rBody["rating"] = score;
            m_networkManager->put(ratingReq, QJsonDocument(rBody).toJson());
        }

        emit updateSuccess(localMangaId);
    });
}

TrackingEntry MangaUpdatesTracker::getEntry(long localMangaId)
{
    TrackingEntry entry;
    entry.mangaId = localMangaId;

    QSettings settings("MihonQT", "MihonQT");
    if (!m_bindings.contains(localMangaId)) {
        long savedRemoteId = settings.value(QString("tracking/mangaupdates_bind_%1").arg(localMangaId), 0).toLongLong();
        if (savedRemoteId > 0) {
            m_bindings[localMangaId] = savedRemoteId;
        }
    }

    if (m_bindings.contains(localMangaId)) {
        entry.id = m_bindings[localMangaId];
        entry.chaptersRead = settings.value(QString("tracking/mangaupdates_entry_chapters_%1").arg(localMangaId), 0).toInt();
        entry.status = settings.value(QString("tracking/mangaupdates_entry_status_%1").arg(localMangaId), Reading).toInt();
        entry.score = settings.value(QString("tracking/mangaupdates_entry_score_%1").arg(localMangaId), 0.0f).toFloat();
    }

    return entry;
}

int MangaUpdatesTracker::mapStatus(int mihonStatus)
{
    // MangaUpdates lists:
    // 0: Reading List
    // 1: Wish List (Plan to Read)
    // 2: Complete List
    // 3: Unfinished List (Dropped)
    // 4: On Hold List
    switch (mihonStatus) {
        case Reading: return 0;
        case PlanToRead: return 1;
        case Completed: return 2;
        case Dropped: return 3;
        case OnHold: return 4;
        default: return 0;
    }
}
