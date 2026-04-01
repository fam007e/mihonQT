#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJSValue> // Needed for Q_INVOKABLE QJSValue return type
#include <QJSEngine> // For QJSEngine reference

class NetworkAccessManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkAccessManager(QJSEngine* engine, QObject *parent = nullptr); // Pass QJSEngine
    
    Q_INVOKABLE QJSValue get(const QString& url);
    Q_INVOKABLE QJSValue post(const QString& url, const QByteArray& data);

    void setTrusted(bool trusted) { m_isTrusted = trusted; }
    void setAllowedBaseUrl(const QString& baseUrl) { m_allowedBaseUrl = baseUrl; }

signals:
    // This signal will be used to pass the result back to JavaScript for asynchronous calls
    // (though current implementation is synchronous).
    void requestFinished(const QJSValue& result);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager m_networkManager;
    QJSEngine* m_engine; // Store the QJSEngine reference
    QString m_userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"; // Default User-Agent matching Chromium
    bool m_isTrusted = true; // Default to true for internal use, will be set to false for untrusted extensions
    QString m_allowedBaseUrl;

    void handleCloudflareIntercept(const QString& url);
    bool isUrlAllowed(const QString& url) const;
};

#endif // NETWORKACCESSMANAGER_H