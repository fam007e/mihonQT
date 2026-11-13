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
    // download and emits loaded(url, pixmap) when it arrives.
    QPixmap load(const QString& url);

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
    QNetworkAccessManager m_nam;
};

#endif // COVERCACHE_H
