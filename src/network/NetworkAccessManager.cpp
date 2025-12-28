#include "NetworkAccessManager.h"
#include <QNetworkRequest>
#include <QEventLoop> // For synchronous network requests (for QJSValue)
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray> // Needed for QJsonArray
#include <QVariant>
#include <QJSValueIterator> // Potentially useful for converting QJSValue to QVariantMap/List

NetworkAccessManager::NetworkAccessManager(QJSEngine* engine, QObject *parent) // Receive QJSEngine
    : QObject(parent)
    , m_engine(engine) // Store engine reference
{
    // It's generally better to connect finished to a slot for asynchronous processing
    // but here we use QEventLoop::exec() for synchronous handling
    // connect(&m_networkManager, &QNetworkAccessManager::finished, this, &NetworkAccessManager::onReplyFinished);
}

QJSValue NetworkAccessManager::get(const QString& url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
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
    QNetworkRequest request(url);
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
