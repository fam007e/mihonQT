#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QMutex>
#include <QMutexLocker>
#include <QJSValue>
#include <QJSEngine>

class SharedCookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit SharedCookieJar(QObject *parent = nullptr) : QNetworkCookieJar(parent) {}

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const override {
        QMutexLocker locker(&m_mutex);
        return QNetworkCookieJar::cookiesForUrl(url);
    }

    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url) override {
        QMutexLocker locker(&m_mutex);
        return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
    }

private:
    mutable QMutex m_mutex;
};

class NetworkAccessManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkAccessManager(QJSEngine* engine = nullptr, QObject *parent = nullptr);
    ~NetworkAccessManager() override;
    
    Q_INVOKABLE QJSValue get(const QString& url);
    Q_INVOKABLE QJSValue post(const QString& url, const QByteArray& data);

    void setTrusted(bool trusted) { m_isTrusted = trusted; }
    void setAllowedBaseUrl(const QString& baseUrl) { m_allowedBaseUrl = baseUrl; }

    static SharedCookieJar* sharedCookieJar();
    // The browser UA used for all extension network requests. The WebView
    // challenge dialog must use the SAME UA: Cloudflare binds cf_clearance
    // to the User-Agent, so a mismatch makes harvested cookies worthless.
    static QString browserUserAgent();

signals:
    void requestFinished(const QJSValue& result);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    // Chrome-navigation header emulation (mirrors keiyoushi BrowserHeaders):
    // many CDNs/WAFs 403 bare non-browser requests before Cloudflare even
    // issues a challenge.
    void applyBrowserHeaders(QNetworkRequest& request) const;
    // 403/503 responses that look like a bot challenge (Cloudflare headers
    // or challenge markers in the body) open the WebView dialog.
    bool looksLikeChallenge(QNetworkReply* reply) const;
    bool solveChallenge(QNetworkReply* reply, const QString& url);

private:
    QNetworkAccessManager m_networkManager;
    QJSEngine* m_engine;
    QString m_userAgent = browserUserAgent();
    bool m_isTrusted = true;
    QString m_allowedBaseUrl;

    void handleCloudflareIntercept(const QString& url);
    bool isUrlAllowed(const QString& url) const;
};

#endif // NETWORKACCESSMANAGER_H
