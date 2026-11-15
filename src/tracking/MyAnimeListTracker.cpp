#include "MyAnimeListTracker.h"
#include "AppSecrets.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QRandomGenerator>
#include <QDebug>

#include <QRegularExpression>

static QString generateCodeVerifier()
{
    // 50 random bytes encoded in URL-safe base64 without padding (~67 chars), matching Mihon Android PkceUtil
    QByteArray bytes;
    bytes.resize(50);
    for (int i = 0; i < 50; ++i) {
        bytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return QString::fromLatin1(bytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

MyAnimeListTracker& MyAnimeListTracker::instance()
{
    static MyAnimeListTracker instance;
    return instance;
}

MyAnimeListTracker::MyAnimeListTracker(QObject *parent)
    : TrackingService(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    QSettings settings("MihonQT", "MihonQT");
    m_accessToken = settings.value("tracking/mal_token", "").toString();
    m_refreshToken = settings.value("tracking/mal_refresh_token", "").toString();
    m_codeVerifier = settings.value("tracking/mal_code_verifier", "").toString();
}

QString MyAnimeListTracker::clientId() const
{
    QSettings settings("MihonQT", "MihonQT");
    QString id = settings.value("tracking/mal_client_id", "").toString().trimmed();
    if (!id.isEmpty()) return id;
    QString secretId = AppSecrets::malClientId().trimmed();
    if (!secretId.isEmpty() && secretId != "@MAL_CLIENT_ID@") return secretId;
    return QString();
}

QString MyAnimeListTracker::redirectUri() const
{
    QSettings settings("MihonQT", "MihonQT");
    return settings.value("tracking/mal_redirect_uri", "http://localhost").toString().trimmed();
}

QUrl MyAnimeListTracker::authUrl() const
{
    QString verifier = generateCodeVerifier();
    const_cast<MyAnimeListTracker*>(this)->m_codeVerifier = verifier;

    QSettings settings("MihonQT", "MihonQT");
    settings.setValue("tracking/mal_code_verifier", verifier);

    QUrl url("https://myanimelist.net/v1/oauth2/authorize");
    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("client_id", clientId());
    query.addQueryItem("code_challenge", verifier);
    query.addQueryItem("code_challenge_method", "plain");
    QString rUri = redirectUri();
    if (!rUri.isEmpty()) {
        query.addQueryItem("redirect_uri", rUri);
    }
    url.setQuery(query);
    return url;
}

bool MyAnimeListTracker::isLoggedIn() const
{
    return !m_accessToken.isEmpty();
}

void MyAnimeListTracker::login(const QString& authCode)
{
    QString code = authCode.trimmed();
    // Support pasting the full URL (e.g. mihonqt://mal-auth?code=... or http://localhost/?code=...)
    if (code.contains("code=")) {
        QUrl u(code);
        QUrlQuery q(u);
        if (q.hasQueryItem("code")) {
            code = q.queryItemValue("code");
        } else {
            static QRegularExpression re(R"(code=([^&]+))");
            auto match = re.match(code);
            if (match.hasMatch()) {
                code = match.captured(1);
            }
        }
    }

    QUrl tokenUrl("https://myanimelist.net/v1/oauth2/token");
    QNetworkRequest request(tokenUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery body;
    body.addQueryItem("client_id", clientId());
    body.addQueryItem("code", code);
    body.addQueryItem("code_verifier", m_codeVerifier);
    body.addQueryItem("grant_type", "authorization_code");
    QString rUri = redirectUri();
    if (!rUri.isEmpty()) {
        body.addQueryItem("redirect_uri", rUri);
    }

    QNetworkReply *reply = m_networkManager->post(request, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed(reply->errorString() + ": " + reply->readAll());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();

        if (root.contains("access_token")) {
            m_accessToken = root["access_token"].toString();
            m_refreshToken = root["refresh_token"].toString();

            QSettings settings("MihonQT", "MihonQT");
            settings.setValue("tracking/mal_token", m_accessToken);
            settings.setValue("tracking/mal_refresh_token", m_refreshToken);

            emit loginSuccess();
        } else {
            emit loginFailed("Failed to parse token response");
        }
    });
}

void MyAnimeListTracker::logout()
{
    m_accessToken.clear();
    m_refreshToken.clear();

    QSettings settings("MihonQT", "MihonQT");
    settings.remove("tracking/mal_token");
    settings.remove("tracking/mal_refresh_token");
    settings.remove("tracking/mal_code_verifier");
}

void MyAnimeListTracker::search(const QString& query)
{
    QUrl url("https://api.myanimelist.net/v2/manga");
    QUrlQuery q;
    q.addQueryItem("q", query.left(64));
    q.addQueryItem("limit", "10");
    q.addQueryItem("fields", "id,title,synopsis,num_chapters,main_picture");
    url.setQuery(q);

    QNetworkRequest request(url);
    if (!m_accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    } else {
        request.setRawHeader("X-MAL-CLIENT-ID", clientId().toUtf8());
    }

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray data = doc.object()["data"].toArray();
        QList<TrackingSearchResult> results;

        for (const QJsonValue& val : data) {
            QJsonObject node = val.toObject()["node"].toObject();
            TrackingSearchResult r;
            r.remoteId = node["id"].toInteger();
            r.title = node["title"].toString();
            r.synopsis = node["synopsis"].toString();
            r.totalChapters = node["num_chapters"].toInt();

            QJsonObject pic = node["main_picture"].toObject();
            r.coverUrl = pic["large"].toString();
            if (r.coverUrl.isEmpty()) {
                r.coverUrl = pic["medium"].toString();
            }

            results.append(r);
        }

        emit searchResults(results);
    });
}

void MyAnimeListTracker::bind(long localMangaId, long remoteId)
{
    m_bindings[localMangaId] = remoteId;

    QSettings settings("MihonQT", "MihonQT");
    settings.setValue(QString("tracking/mal_bind_%1").arg(localMangaId), static_cast<qlonglong>(remoteId));

    emit bindSuccess(localMangaId);
}

void MyAnimeListTracker::unbind(long localMangaId)
{
    m_bindings.remove(localMangaId);

    QSettings settings("MihonQT", "MihonQT");
    settings.remove(QString("tracking/mal_bind_%1").arg(localMangaId));
    settings.remove(QString("tracking/mal_entry_chapters_%1").arg(localMangaId));
    settings.remove(QString("tracking/mal_entry_status_%1").arg(localMangaId));
    settings.remove(QString("tracking/mal_entry_score_%1").arg(localMangaId));
}

void MyAnimeListTracker::update(long localMangaId, int chaptersRead, int status, float score)
{
    if (!m_bindings.contains(localMangaId)) {
        QSettings settings("MihonQT", "MihonQT");
        long savedId = settings.value(QString("tracking/mal_bind_%1").arg(localMangaId), 0).toLongLong();
        if (savedId > 0) {
            m_bindings[localMangaId] = savedId;
        } else {
            emit error("Manga not bound to MyAnimeList");
            return;
        }
    }

    if (m_accessToken.isEmpty()) {
        emit error("Not logged in to MyAnimeList");
        return;
    }

    long remoteId = m_bindings[localMangaId];
    QUrl url(QString("https://api.myanimelist.net/v2/manga/%1/my_list_status").arg(remoteId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());

    QUrlQuery body;
    body.addQueryItem("status", mapStatus(status));
    body.addQueryItem("num_chapters_read", QString::number(chaptersRead));
    if (score > 0) {
        body.addQueryItem("score", QString::number(static_cast<int>(score)));
    }

    QNetworkReply *reply = m_networkManager->put(request, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply, localMangaId, chaptersRead, status, score]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString() + ": " + reply->readAll());
            return;
        }

        QSettings settings("MihonQT", "MihonQT");
        settings.setValue(QString("tracking/mal_entry_chapters_%1").arg(localMangaId), chaptersRead);
        settings.setValue(QString("tracking/mal_entry_status_%1").arg(localMangaId), status);
        settings.setValue(QString("tracking/mal_entry_score_%1").arg(localMangaId), score);

        emit updateSuccess(localMangaId);
    });
}

TrackingEntry MyAnimeListTracker::getEntry(long localMangaId)
{
    TrackingEntry entry;
    entry.mangaId = localMangaId;

    QSettings settings("MihonQT", "MihonQT");
    if (!m_bindings.contains(localMangaId)) {
        long savedRemoteId = settings.value(QString("tracking/mal_bind_%1").arg(localMangaId), 0).toLongLong();
        if (savedRemoteId > 0) {
            m_bindings[localMangaId] = savedRemoteId;
        }
    }

    if (m_bindings.contains(localMangaId)) {
        entry.id = m_bindings[localMangaId];
        entry.chaptersRead = settings.value(QString("tracking/mal_entry_chapters_%1").arg(localMangaId), 0).toInt();
        entry.status = settings.value(QString("tracking/mal_entry_status_%1").arg(localMangaId), Reading).toInt();
        entry.score = settings.value(QString("tracking/mal_entry_score_%1").arg(localMangaId), 0.0f).toFloat();
    }

    return entry;
}

QString MyAnimeListTracker::mapStatus(int mihonStatus)
{
    switch (mihonStatus) {
        case Reading: return "reading";
        case Completed: return "completed";
        case OnHold: return "on_hold";
        case Dropped: return "dropped";
        case PlanToRead: return "plan_to_read";
        default: return "reading";
    }
}
