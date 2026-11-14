#include "NetworkAccessManager.h"
#include <QNetworkRequest>
#include <QEventLoop>
#include <QDebug>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include "config/PreferenceManager.h"
#include <QVariant>
#include <QApplication>
#include "../ui/WebViewDialog.h"

SharedCookieJar* NetworkAccessManager::sharedCookieJar()
{
    static SharedCookieJar* s_jar = new SharedCookieJar(qApp);
    return s_jar;
}

QString NetworkAccessManager::browserUserAgent()
{
    static const QString ua = QStringLiteral(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    return ua;
}

NetworkAccessManager::NetworkAccessManager(QJSEngine* engine, QObject *parent) 
    : QObject(parent)
    , m_engine(engine) 
{
    m_networkManager.setCookieJar(sharedCookieJar());
    // Prevent QNAM from taking ownership of the shared jar.
    // setCookieJar() reparents the jar to the QNAM, so when this QNAM
    // destructs it would delete the shared singleton. Reset parent to qApp.
    sharedCookieJar()->setParent(qApp);
}

NetworkAccessManager::~NetworkAccessManager()
{
    m_networkManager.setCookieJar(nullptr);
}

QJSValue NetworkAccessManager::get(const QString& url)
{
    if (!isUrlAllowed(url)) {
        if (m_engine) {
            return m_engine->newErrorObject(QJSValue::GenericError, "Requested URL is not allowed: " + url);
        }
        return QJSValue();
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
            if (doc.isObject() && m_engine) {
                result = m_engine->toScriptValue(doc.object().toVariantMap());
            } else if (doc.isArray() && m_engine) {
                result = m_engine->toScriptValue(doc.array().toVariantList());
            } else if (m_engine) {
                result = m_engine->toScriptValue(QString::fromUtf8(data));
            }
        } else if (m_engine) {
            result = m_engine->toScriptValue(QString::fromUtf8(data));
        }
    } else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 503 && reply->rawHeader("Server").toLower().contains("cloudflare")) {
            qWarning() << "Cloudflare GET challenge detected! Opening WebView Interceptor...";
            
            bool success = false;
            auto showDialog = [url, &success]() {
                WebViewDialog dialog(url);
                dialog.exec();
                
                QList<QNetworkCookie> cookies = dialog.getHarvestedCookies();
                if (!cookies.isEmpty()) {
                    sharedCookieJar()->setCookiesFromUrl(cookies, QUrl(url));
                    success = true;
                }
            };

            if (QThread::currentThread() == qApp->thread()) {
                showDialog();
            } else {
                QMetaObject::invokeMethod(qApp, showDialog, Qt::BlockingQueuedConnection);
            }

            if (success) {
                reply->deleteLater();
                return this->get(url); // Retry the request with the new cookies
            }
        }

        qWarning() << "Network GET request failed:" << reply->errorString();
        if (m_engine) {
            QJsonObject errorObject;
            errorObject["error"] = reply->errorString();
            result = m_engine->toScriptValue(errorObject.toVariantMap());
        }
    }

    reply->deleteLater();
    return result;
}

QJSValue NetworkAccessManager::post(const QString& url, const QByteArray& data)
{
    if (!isUrlAllowed(url)) {
        if (m_engine) {
            return m_engine->newErrorObject(QJSValue::GenericError, "Requested URL is not allowed: " + url);
        }
        return QJSValue();
    }

    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    QNetworkReply *reply = m_networkManager.post(request, data);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QJSValue result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            if (doc.isObject() && m_engine) {
                result = m_engine->toScriptValue(doc.object().toVariantMap());
            } else if (doc.isArray() && m_engine) {
                result = m_engine->toScriptValue(doc.array().toVariantList());
            } else if (m_engine) {
                result = m_engine->toScriptValue(QString::fromUtf8(responseData));
            }
        } else if (m_engine) {
            result = m_engine->toScriptValue(QString::fromUtf8(responseData));
        }
    } else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 503 && reply->rawHeader("Server").toLower().contains("cloudflare")) {
            qWarning() << "Cloudflare POST challenge detected! Opening WebView Interceptor...";
            bool success = false;
            auto showDialog = [url, &success]() {
                WebViewDialog dialog(url);
                dialog.exec();
                QList<QNetworkCookie> cookies = dialog.getHarvestedCookies();
                if (!cookies.isEmpty()) {
                    sharedCookieJar()->setCookiesFromUrl(cookies, QUrl(url));
                    success = true;
                }
            };

            if (QThread::currentThread() == qApp->thread()) {
                showDialog();
            } else {
                QMetaObject::invokeMethod(qApp, showDialog, Qt::BlockingQueuedConnection);
            }

            if (success) {
                reply->deleteLater();
                return this->post(url, data);
            }
        }

        qWarning() << "Network POST request failed:" << reply->errorString();
        if (m_engine) {
            QJsonObject errorObject;
            errorObject["error"] = reply->errorString();
            result = m_engine->toScriptValue(errorObject.toVariantMap());
        }
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
