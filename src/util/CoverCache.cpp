#include "CoverCache.h"
#include "ImageUtil.h"
#include "../network/NetworkAccessManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCryptographicHash>

CoverCache& CoverCache::instance()
{
    static CoverCache inst;
    return inst;
}

CoverCache::CoverCache(QObject *parent)
    : QObject(parent)
{
}

QString CoverCache::cacheDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/covers";
    QDir().mkpath(dir);
    return dir;
}

QString coverKey(const QString& url)
{
    return QString::fromLatin1(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex());
}

QPixmap CoverCache::loadFromDisk(const QString& key)
{
    QPixmap pm;
    pm.load(cacheDir() + "/" + key + ".img");
    return pm;
}

void CoverCache::saveToDisk(const QString& key, const QByteArray& data)
{
    QFile f(cacheDir() + "/" + key + ".img");
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

void CoverCache::clearMemoryCache()
{
    m_memory.clear();
}

QPixmap CoverCache::load(const QString& url, const QString& referer)
{
    if (url.isEmpty())
        return {};

    // Memory cache
    auto it = m_memory.constFind(url);
    if (it != m_memory.constEnd())
        return it.value();

    // Disk cache
    QString key = coverKey(url);
    QPixmap disk = loadFromDisk(key);
    if (!disk.isNull()) {
        m_memory.insert(url, disk);
        return disk;
    }

    // Fetch once (guard against duplicate requests while a grid rebuilds)
    if (m_inFlight.value(url, false))
        return {};
    m_inFlight.insert(url, true);

    // Image CDNs with hotlink protection (pstatic, madara cover CDNs, ...)
    // 403 requests that don't look like a browser image load from the site.
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, NetworkAccessManager::browserUserAgent());
    request.setRawHeader("Accept", "image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    request.setRawHeader("sec-ch-ua", "\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Google Chrome\";v=\"120\"");
    request.setRawHeader("sec-ch-ua-mobile", "?0");
    request.setRawHeader("sec-ch-ua-platform", "\"Windows\"");
    request.setRawHeader("Sec-Fetch-Dest", "image");
    request.setRawHeader("Sec-Fetch-Mode", "no-cors");
    request.setRawHeader("Sec-Fetch-Site", "cross-site");
    if (!referer.isEmpty()) {
        request.setRawHeader("Referer", referer.toUtf8());
        request.setRawHeader("Sec-Fetch-Site", "same-site");
    }
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    // A grid fires ~20 parallel cover fetches at one CDN; HTTP/2 multiplexing
    // gets streams refused (REFUSED_STREAM). Plain HTTP/1.1 connections and
    // one retry below handle it.
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QNetworkReply *reply = m_nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url, key]() {
        reply->deleteLater();
        m_inFlight.remove(url);
        if (reply->error() != QNetworkReply::NoError) {
            // Up to 2 retries for transient errors (HTTP/2 REFUSED_STREAM,
            // resets); hard refusals like 403 are not retried.
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const int attempt = m_attempts.value(url, 0) + 1;
            if (status != 403 && status != 410 && attempt <= 2 && !m_inFlight.value(url, false)) {
                m_attempts.insert(url, attempt);
                qDebug() << "Retrying cover download (" << attempt << "):" << url;
                load(url); // re-enters the normal cache/in-flight path
            } else {
                m_attempts.remove(url);
                qWarning() << "Cover download failed:" << url << reply->errorString();
            }
            return;
        }
        m_attempts.remove(url);
        QByteArray data = reply->readAll();
        QPixmap pm = ImageUtil::loadPixmapFromData(data);
        if (pm.isNull()) {
            qWarning() << "Failed to decode cover image:" << url;
            return;
        }
        m_memory.insert(url, pm);
        saveToDisk(key, data);
        emit loaded(url, pm);
    });

    return {};
}
