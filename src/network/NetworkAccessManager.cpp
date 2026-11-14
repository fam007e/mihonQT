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

void NetworkAccessManager::applyBrowserHeaders(QNetworkRequest& request) const
{
    // Top-level Chrome navigation headers; sites behind CDNs/WAFs 403
    // requests that lack the client hints and fetch metadata a browser sends.
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    request.setRawHeader("Upgrade-Insecure-Requests", "1");
    request.setRawHeader("sec-ch-ua", "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Google Chrome\";v=\"120\"");
    request.setRawHeader("sec-ch-ua-mobile", "?0");
    request.setRawHeader("sec-ch-ua-platform", "\"Windows\"");
    request.setRawHeader("Sec-Fetch-Dest", "document");
    request.setRawHeader("Sec-Fetch-Mode", "navigate");
    request.setRawHeader("Sec-Fetch-Site", "none");
    request.setRawHeader("Sec-Fetch-User", "?1");
}

bool NetworkAccessManager::looksLikeChallenge(QNetworkReply* reply) const
{
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 403 && statusCode != 503) return false;

    if (reply->rawHeader("Server").toLower().contains("cloudflare")) return true;
    // cf-mitigated: challenge is the canonical Cloudflare signal
    if (reply->rawHeader("cf-mitigated").toLower().contains("challenge")) return true;

    // Body sniff for challenges that don't advertise themselves in headers
    const QString body = QString::fromUtf8(reply->peek(65536));
    return body.contains("Just a moment", Qt::CaseInsensitive) ||
           body.contains("Attention Required", Qt::CaseInsensitive) ||
           body.contains("cf-chl", Qt::CaseInsensitive) ||
           body.contains("challenge-platform", Qt::CaseInsensitive);
}

bool NetworkAccessManager::solveChallenge(QNetworkReply* reply, const QString& url)
{
    if (!looksLikeChallenge(reply)) return false;

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qWarning() << "Bot challenge detected (" << statusCode << ") on" << url << "- opening WebView interceptor...";

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
    return success;
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
    applyBrowserHeaders(request);
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
        if (solveChallenge(reply, url)) {
            reply->deleteLater();
            return this->get(url); // Retry the request with the new cookies
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
    applyBrowserHeaders(request);
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
        if (solveChallenge(reply, url)) {
            reply->deleteLater();
            return this->post(url, data);
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
