#include "NetworkAccessManager.h"
#include <QNetworkRequest>
#include <QEventLoop> // For synchronous network requests (for QJSValue)
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray> // Needed for QJsonArray
#include <QUrl>
#include "config/PreferenceManager.h"
#include <QVariant>
#include <QJSValueIterator> 
#include <QNetworkCookieJar>
#include <QApplication>
#include "../ui/WebViewDialog.h"

NetworkAccessManager::NetworkAccessManager(QJSEngine* engine, QObject *parent) 
    : QObject(parent)
    , m_engine(engine) 
{
    m_networkManager.setCookieJar(new QNetworkCookieJar(this));
}

QJSValue NetworkAccessManager::get(const QString& url)
{
    if (!isUrlAllowed(url)) {
        return m_engine->newErrorObject(QJSValue::GenericError, "Requested URL is not allowed: " + url);
    }
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    QNetworkReply *reply = m_networkManager.get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // Blocks until the reply finishes

    // Process the reply synchronously for returning QJSValue
    QJSValue result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        // Try to parse as JSON, otherwise return as string
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                result = m_engine->toScriptValue(doc.object().toVariantMap()); // Convert QJsonObject to QVariantMap
            } else if (doc.isArray()) {
                result = m_engine->toScriptValue(doc.array().toVariantList()); // Convert QJsonArray to QVariantList
            } else {
                result = m_engine->toScriptValue(QString(data)); // Fallback to string if not JSON object/array
            }
        } else {
            result = m_engine->toScriptValue(QString(data)); // If JSON parsing fails, return as plain string
        }
    } else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 503 && reply->rawHeader("Server").toLower().contains("cloudflare")) {
            qWarning() << "Cloudflare GET challenge detected! Opening WebView Interceptor...";
            
            bool success = false;
            // Ensure dialog runs on main thread
            QMetaObject::invokeMethod(qApp, [this, url, &success]() {
                WebViewDialog dialog(url);
                dialog.exec();
                
                QList<QNetworkCookie> cookies = dialog.getHarvestedCookies();
                if (!cookies.isEmpty()) {
                    m_networkManager.cookieJar()->setCookiesFromUrl(cookies, QUrl(url));
                    success = true;
                }
            }, Qt::BlockingQueuedConnection);

            if (success) {
                reply->deleteLater();
                return this->get(url); // Retry the request with the new cookies
            }
        }

        qWarning() << "Network GET request failed:" << reply->errorString();
        QJsonObject errorObject;
        errorObject["error"] = reply->errorString();
        result = m_engine->toScriptValue(errorObject.toVariantMap()); // Convert QJsonObject error to QVariantMap
    }

    reply->deleteLater();
    return result;
}

QJSValue NetworkAccessManager::post(const QString& url, const QByteArray& data)
{
    if (!isUrlAllowed(url)) {
        return m_engine->newErrorObject(QJSValue::GenericError, "Requested URL is not allowed: " + url);
    }
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); // Example: assuming JSON post
    QNetworkReply *reply = m_networkManager.post(request, data);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // Blocks until the reply finishes

    QJSValue result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                result = m_engine->toScriptValue(doc.object().toVariantMap());
            } else if (doc.isArray()) {
                result = m_engine->toScriptValue(doc.array().toVariantList());
            } else {
                result = m_engine->toScriptValue(QString(responseData));
            }
        } else {
            result = m_engine->toScriptValue(QString(responseData));
        }
    } else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 503 && reply->rawHeader("Server").toLower().contains("cloudflare")) {
            qWarning() << "Cloudflare POST challenge detected! Opening WebView Interceptor...";
            bool success = false;
            QMetaObject::invokeMethod(qApp, [this, url, &success]() {
                WebViewDialog dialog(url);
                dialog.exec();
                QList<QNetworkCookie> cookies = dialog.getHarvestedCookies();
                if (!cookies.isEmpty()) {
                    m_networkManager.cookieJar()->setCookiesFromUrl(cookies, QUrl(url));
                    success = true;
                }
            }, Qt::BlockingQueuedConnection);

            if (success) {
                reply->deleteLater();
                return this->post(url, data); // Retry
            }
        }

        qWarning() << "Network POST request failed:" << reply->errorString();
        QJsonObject errorObject;
        errorObject["error"] = reply->errorString();
        result = m_engine->toScriptValue(errorObject.toVariantMap());
    }

    reply->deleteLater();
    return result;
}

void NetworkAccessManager::onReplyFinished(QNetworkReply* reply)
{
    // This slot is connected to m_networkManager.finished,
    // but our QJSValue returning methods use QEventLoop for synchronous processing.
    // So this might not be directly used for returning values to JS in this synchronous model.
    // It could be used for logging or general error handling.
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Asynchronous reply error (not caught by QJSValue methods):" << reply->errorString();
    }
    // reply->deleteLater(); // Handled by the QJSValue methods
}

void NetworkAccessManager::handleCloudflareIntercept(const QString& url)
{
    // Implementation for Cloudflare handling
    qDebug() << "Cloudflare intercept for:" << url;
}

bool NetworkAccessManager::isUrlAllowed(const QString& urlString) const
{
    QUrl url(urlString);
    
    // Check HTTPS enforcement
    bool enforceHttps = PreferenceManager::instance().value(PreferenceManager::ENFORCE_HTTPS, false).toBool();
    if (enforceHttps && url.scheme().toLower() == "http") {
        qWarning() << "Blocking insecure request due to HTTPS enforcement:" << urlString;
        return false;
    }

    // Check base URL restriction for untrusted extensions
    if (!m_isTrusted && !m_allowedBaseUrl.isEmpty()) {
        QUrl baseUrl(m_allowedBaseUrl);
        if (url.host().toLower() != baseUrl.host().toLower()) {
            qWarning() << "Blocking cross-domain request from untrusted extension:" << urlString 
                       << "(Allowed domain:" << baseUrl.host() << ")";
            return false;
        }
    }

    return true;
}
