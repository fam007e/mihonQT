#include "AniListTracker.h"
#include "AppSecrets.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QDebug>

const QString AniListTracker::REDIRECT_URI = "https://anilist.co/api/v2/oauth/pin";

AniListTracker& AniListTracker::instance()
{
    static AniListTracker instance;
    return instance;
}

AniListTracker::AniListTracker(QObject *parent)
    : TrackingService(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    // Load saved token
    QSettings settings("MihonQT", "MihonQT");
    m_accessToken = settings.value("tracking/anilist_token", "").toString();
    m_userId = settings.value("tracking/anilist_user_id", 0).toLongLong();
}

QString AniListTracker::clientId() const
{
    QSettings settings("MihonQT", "MihonQT");
    QString id = settings.value("tracking/anilist_client_id", "").toString().trimmed();
    if (!id.isEmpty()) return id;
    QString secretId = AppSecrets::anilistClientId().trimmed();
    if (!secretId.isEmpty() && secretId != "@ANILIST_CLIENT_ID@") return secretId;
    return QString();
}

QUrl AniListTracker::authUrl() const
{
    QUrl url("https://anilist.co/api/v2/oauth/authorize");
    QUrlQuery query;
    query.addQueryItem("client_id", clientId());
    query.addQueryItem("response_type", "token");
    url.setQuery(query);
    return url;
}

bool AniListTracker::isLoggedIn() const
{
    return !m_accessToken.isEmpty();
}

void AniListTracker::login(const QString& authCode)
{
    // For implicit grant, authCode is the access_token from URL fragment
    m_accessToken = authCode;

    // Get user ID
    QString query = R"(
        query {
            Viewer {
                id
                name
            }
        }
    )";

    executeGraphQL(query, QJsonObject(), [this](const QJsonObject& data) {
        if (data.contains("Viewer")) {
            m_userId = data["Viewer"].toObject()["id"].toInteger();

            QSettings settings("MihonQT", "MihonQT");
            settings.setValue("tracking/anilist_token", m_accessToken);
            settings.setValue("tracking/anilist_user_id", static_cast<qlonglong>(m_userId));

            emit loginSuccess();
        } else {
            emit loginFailed("Failed to get user info");
        }
    });
}

void AniListTracker::logout()
{
    m_accessToken.clear();
    m_userId = 0;

    QSettings settings("MihonQT", "MihonQT");
    settings.remove("tracking/anilist_token");
    settings.remove("tracking/anilist_user_id");
}

void AniListTracker::search(const QString& query)
{
    QString gql = R"(
        query ($search: String) {
            Page(perPage: 10) {
                media(search: $search, type: MANGA) {
                    id
                    title { romaji english native }
                    coverImage { large }
                    description
                    chapters
                }
            }
        }
    )";

    QJsonObject vars;
    vars["search"] = query;

    executeGraphQL(gql, vars, [this](const QJsonObject& data) {
        QList<TrackingSearchResult> results;

        if (data.contains("Page")) {
            QJsonArray media = data["Page"].toObject()["media"].toArray();
            for (const QJsonValue& val : media) {
                QJsonObject m = val.toObject();
                TrackingSearchResult r;
                r.remoteId = m["id"].toInteger();

                QJsonObject title = m["title"].toObject();
                r.title = title["english"].toString();
                if (r.title.isEmpty()) r.title = title["romaji"].toString();

                r.coverUrl = m["coverImage"].toObject()["large"].toString();
                r.synopsis = m["description"].toString();
                r.totalChapters = m["chapters"].toInt();

                results.append(r);
            }
        }

        emit searchResults(results);
    });
}

void AniListTracker::bind(long localMangaId, long remoteId)
{
    m_bindings[localMangaId] = remoteId;

    // Save bindings
    QSettings settings("MihonQT", "MihonQT");
    settings.setValue(QString("tracking/anilist_bind_%1").arg(localMangaId), static_cast<qlonglong>(remoteId));

    emit bindSuccess(localMangaId);
}

void AniListTracker::unbind(long localMangaId)
{
    m_bindings.remove(localMangaId);

    QSettings settings("MihonQT", "MihonQT");
    settings.remove(QString("tracking/anilist_bind_%1").arg(localMangaId));
    settings.remove(QString("tracking/anilist_entry_chapters_%1").arg(localMangaId));
    settings.remove(QString("tracking/anilist_entry_status_%1").arg(localMangaId));
    settings.remove(QString("tracking/anilist_entry_score_%1").arg(localMangaId));
}

void AniListTracker::update(long localMangaId, int chaptersRead, int status, float score)
{
    if (!m_bindings.contains(localMangaId)) {
        emit error("Manga not bound to AniList");
        return;
    }

    long mediaId = m_bindings[localMangaId];

    QString gql = R"(
        mutation ($mediaId: Int, $progress: Int, $status: MediaListStatus, $score: Float) {
            SaveMediaListEntry(mediaId: $mediaId, progress: $progress, status: $status, score: $score) {
                id
                progress
                status
                score
            }
        }
    )";

    QJsonObject vars;
    vars["mediaId"] = static_cast<int>(mediaId);
    vars["progress"] = chaptersRead;
    vars["status"] = mapStatus(status);
    if (score > 0) vars["score"] = static_cast<double>(score);

    executeGraphQL(gql, vars, [this, localMangaId, chaptersRead, status, score](const QJsonObject& data) {
        if (data.contains("SaveMediaListEntry")) {
            QSettings settings("MihonQT", "MihonQT");
            settings.setValue(QString("tracking/anilist_entry_chapters_%1").arg(localMangaId), chaptersRead);
            settings.setValue(QString("tracking/anilist_entry_status_%1").arg(localMangaId), status);
            settings.setValue(QString("tracking/anilist_entry_score_%1").arg(localMangaId), score);
            emit updateSuccess(localMangaId);
        } else {
            emit error("Failed to update AniList");
        }
    });
}

TrackingEntry AniListTracker::getEntry(long localMangaId)
{
    TrackingEntry entry;
    entry.mangaId = localMangaId;

    QSettings settings("MihonQT", "MihonQT");
    if (!m_bindings.contains(localMangaId)) {
        long savedRemoteId = settings.value(QString("tracking/anilist_bind_%1").arg(localMangaId), 0).toLongLong();
        if (savedRemoteId > 0) {
            m_bindings[localMangaId] = savedRemoteId;
        }
    }

    if (m_bindings.contains(localMangaId)) {
        entry.id = m_bindings[localMangaId];
        entry.chaptersRead = settings.value(QString("tracking/anilist_entry_chapters_%1").arg(localMangaId), 0).toInt();
        entry.status = settings.value(QString("tracking/anilist_entry_status_%1").arg(localMangaId), Reading).toInt();
        entry.score = settings.value(QString("tracking/anilist_entry_score_%1").arg(localMangaId), 0.0f).toFloat();
    }

    return entry;
}

QString AniListTracker::mapStatus(int mihonStatus)
{
    // AniList GraphQL MediaListStatus enum values
    switch (mihonStatus) {
        case Reading: return "CURRENT";
        case Completed: return "COMPLETED";
        case OnHold: return "PAUSED";
        case Dropped: return "DROPPED";
        case PlanToRead: return "PLANNING";
        default: return "CURRENT";
    }
}

void AniListTracker::executeGraphQL(const QString& query, const QJsonObject& variables,
                                     std::function<void(const QJsonObject&)> callback)
{
    QNetworkRequest request(QUrl("https://graphql.anilist.co"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    }

    QJsonObject body;
    body["query"] = query;
    if (!variables.isEmpty()) {
        body["variables"] = variables;
    }

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();

        if (root.contains("errors")) {
            QString errMsg = root["errors"].toArray().first().toObject()["message"].toString();
            emit error(errMsg);
            return;
        }

        if (root.contains("data")) {
            callback(root["data"].toObject());
        }
    });
}
