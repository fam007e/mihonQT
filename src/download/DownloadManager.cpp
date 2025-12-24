#include "DownloadManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>
#include <QImageReader>
#include <QFile>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

DownloadManager& DownloadManager::instance()
{
    static DownloadManager instance;
    return instance;
}

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
    // Default download path
    m_downloadPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
    QDir().mkpath(m_downloadPath);
}

QString DownloadManager::getDownloadPath() const
{
    return m_downloadPath;
}

void DownloadManager::setDownloadPath(const QString& path)
{
    m_downloadPath = path;
    QDir().mkpath(m_downloadPath);
}

void DownloadManager::queueDownload(const Manga& manga, const Chapter& chapter)
{
    QMutexLocker locker(&m_mutex);

    // Check if already in queue
    for (const auto& item : m_queue) {
        if (item.chapterId == chapter.id()) {
            qDebug() << "Chapter already in download queue:" << chapter.name();
            return;
        }
    }

    DownloadItem item;
    item.mangaId = manga.id();
    item.chapterId = chapter.id();
    item.mangaTitle = manga.title();
    item.chapterName = chapter.name();
    item.chapterUrl = chapter.url();

    // Create destination path: downloads/MangaTitle/ChapterName/
    QString sanitizedManga = manga.title();
    sanitizedManga.replace(QRegularExpression("[<>:\"/\\|?*]"), "_");
    QString sanitizedChapter = chapter.name();
    sanitizedChapter.replace(QRegularExpression("[<>:\"/\\|?*]"), "_");

    item.destinationPath = m_downloadPath + "/" + sanitizedManga + "/" + sanitizedChapter;

    m_queue.enqueue(item);
    emit downloadQueued(item);
    emit queueChanged();

    if (!m_isProcessing) {
        processQueue();
    }
}

void DownloadManager::pauseDownload(long chapterId)
{
    // For MVP, just mark as not downloading (actual pause logic would require more complex state)
    QMutexLocker locker(&m_mutex);
    if (m_activeDownloads.contains(chapterId)) {
        m_activeDownloads[chapterId].isDownloading = false;
    }
}

void DownloadManager::resumeDownload(long chapterId)
{
    QMutexLocker locker(&m_mutex);
    if (m_activeDownloads.contains(chapterId)) {
        m_activeDownloads[chapterId].isDownloading = true;
    }
    if (!m_isProcessing) {
        processQueue();
    }
}

void DownloadManager::cancelDownload(long chapterId)
{
    QMutexLocker locker(&m_mutex);

    // Remove from queue
    QQueue<DownloadItem> newQueue;
    while (!m_queue.isEmpty()) {
        DownloadItem item = m_queue.dequeue();
        if (item.chapterId != chapterId) {
            newQueue.enqueue(item);
        }
    }
    m_queue = newQueue;

    // Remove from active
    m_activeDownloads.remove(chapterId);
    emit queueChanged();
}

void DownloadManager::cancelAllDownloads()
{
    QMutexLocker locker(&m_mutex);
    m_queue.clear();
    m_activeDownloads.clear();
    m_isProcessing = false;
    emit queueChanged();
}

QList<DownloadItem> DownloadManager::getQueue() const
{
    QMutexLocker locker(&m_mutex);
    return m_queue.toList();
}

DownloadItem DownloadManager::getCurrentDownload() const
{
    QMutexLocker locker(&m_mutex);
    if (!m_activeDownloads.isEmpty()) {
        return m_activeDownloads.first();
    }
    return DownloadItem();
}

bool DownloadManager::isDownloading() const
{
    return m_isProcessing;
}

void DownloadManager::processQueue()
{
    QMutexLocker locker(&m_mutex);

    if (m_queue.isEmpty()) {
        m_isProcessing = false;
        return;
    }

    m_isProcessing = true;
    DownloadItem item = m_queue.dequeue();
    item.isDownloading = true;
    m_activeDownloads[item.chapterId] = item;

    locker.unlock();

    emit downloadStarted(item.chapterId);

    // Run download in background thread
    QThreadPool::globalInstance()->start([this, item]() mutable {
        downloadChapter(item);
    });
}

void DownloadManager::downloadChapter(DownloadItem& item)
{
    qDebug() << "DownloadManager: Downloading chapter" << item.chapterName << "to" << item.destinationPath;

    QDir().mkpath(item.destinationPath);

    QString chapterUrl = item.chapterUrl;
    QStringList pageUrls;

    // Extract page URLs based on chapter type (CBZ or directory)
    if (chapterUrl.startsWith("cbz://")) {
        chapterUrl.remove(0, 6);
        QuaZip zip(chapterUrl);
        if (zip.open(QuaZip::mdUnzip)) {
            QStringList files = zip.getFileNameList();
            for (const QString& f : files) {
                QFileInfo fi(f);
                QString ext = fi.suffix().toLower();
                if (ext == "jpg" || ext == "png" || ext == "jpeg" || ext == "webp") {
                    pageUrls.append(QString("cbz://%1/%2").arg(chapterUrl, f));
                }
            }
            zip.close();
        }
    } else {
        QDir dir(chapterUrl);
        QFileInfoList entries = dir.entryInfoList(QDir::Files, QDir::Name);
        for (const QFileInfo& entry : entries) {
            QString ext = entry.suffix().toLower();
            if (ext == "jpg" || ext == "png" || ext == "jpeg" || ext == "webp") {
                pageUrls.append(entry.absoluteFilePath());
            }
        }
    }

    std::sort(pageUrls.begin(), pageUrls.end());
    item.totalPages = pageUrls.size();

    // Copy each page
    int pageNum = 0;
    for (const QString& pageUrl : pageUrls) {
        QString destFile = item.destinationPath + QString("/%1.jpg").arg(pageNum, 4, 10, QChar('0'));

        QImage image;
        if (pageUrl.startsWith("cbz://")) {
            QString url = pageUrl.mid(6);
            int cbzIdx = url.indexOf(".cbz/", 0, Qt::CaseInsensitive);
            if (cbzIdx != -1) {
                QString archivePath = url.left(cbzIdx + 4);
                QString imagePath = url.mid(cbzIdx + 5);

                QuaZip zip(archivePath);
                if (zip.open(QuaZip::mdUnzip) && zip.setCurrentFile(imagePath)) {
                    QuaZipFile file(&zip);
                    if (file.open(QIODevice::ReadOnly)) {
                        image.loadFromData(file.readAll());
                        file.close();
                    }
                    zip.close();
                }
            }
        } else {
            image.load(pageUrl);
        }

        if (!image.isNull()) {
            image.save(destFile, "JPEG", 90);
        }

        pageNum++;
        item.downloadedPages = pageNum;

        QMetaObject::invokeMethod(this, [this, chapterId = item.chapterId, current = pageNum, total = item.totalPages]() {
            emit downloadProgress(chapterId, current, total);
        }, Qt::QueuedConnection);
    }

    // Mark complete
    {
        QMutexLocker locker(&m_mutex);
        m_activeDownloads.remove(item.chapterId);
    }

    QMetaObject::invokeMethod(this, [this, chapterId = item.chapterId]() {
        emit downloadComplete(chapterId);
        emit queueChanged();
        processQueue();
    }, Qt::QueuedConnection);
}
