#ifndef COVERCACHE_H
#define COVERCACHE_H

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QNetworkAccessManager>

// Shared cover loader with memory + disk caching (mirrors Mihon's cover caching:
// one network fetch per cover, persisted under the user cache dir).
class CoverCache : public QObject
{
    Q_OBJECT
public:
    static CoverCache& instance();

    // Returns the cover if cached (memory or disk). Otherwise starts an async
    // download and emits loaded(url, pixmap) when it arrives. `referer` is
    // the calling site's page origin — image CDNs with hotlink protection
    // (e.g. webtoon-phinf.pstatic.net) 403 requests without it.
    QPixmap load(const QString& url, const QString& referer = {});

    void clearMemoryCache();

signals:
    void loaded(const QString& url, const QPixmap& pixmap);

private:
    explicit CoverCache(QObject *parent = nullptr);

    QPixmap loadFromDisk(const QString& key);
    void saveToDisk(const QString& key, const QByteArray& data);
    QString cacheDir() const;

    QHash<QString, QPixmap> m_memory;
    QHash<QString, bool> m_inFlight;
    QHash<QString, int> m_attempts;
    QNetworkAccessManager m_nam;
};

#endif // COVERCACHE_H
