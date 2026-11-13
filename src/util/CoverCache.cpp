#include "CoverCache.h"
#include "ImageUtil.h"
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

QPixmap CoverCache::load(const QString& url)
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

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader,
        QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                       "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url, key]() {
        reply->deleteLater();
        m_inFlight.remove(url);
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Cover download failed:" << url << reply->errorString();
            return;
        }
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
