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

signals:
    void requestFinished(const QJSValue& result);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager m_networkManager;
    QJSEngine* m_engine;
    QString m_userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
    bool m_isTrusted = true;
    QString m_allowedBaseUrl;

    void handleCloudflareIntercept(const QString& url);
    bool isUrlAllowed(const QString& url) const;
};

#endif // NETWORKACCESSMANAGER_H
