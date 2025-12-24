#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QQueue>
#include <QMap>
#include <QMutex>
#include <QThread>
#include "../model/Manga.h"
#include "../model/Chapter.h"

struct DownloadItem {
    long mangaId;
    long chapterId;
    QString mangaTitle;
    QString chapterName;
    QString chapterUrl;
    QString destinationPath;
    int totalPages = 0;
    int downloadedPages = 0;
    bool isDownloading = false;
    bool isComplete = false;
    bool hasFailed = false;
};

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    static DownloadManager& instance();

    void queueDownload(const Manga& manga, const Chapter& chapter);
    void pauseDownload(long chapterId);
    void resumeDownload(long chapterId);
    void cancelDownload(long chapterId);
    void cancelAllDownloads();

    QList<DownloadItem> getQueue() const;
    DownloadItem getCurrentDownload() const;
    bool isDownloading() const;

    QString getDownloadPath() const;
    void setDownloadPath(const QString& path);

signals:
    void downloadQueued(const DownloadItem& item);
    void downloadStarted(long chapterId);
    void downloadProgress(long chapterId, int current, int total);
    void downloadComplete(long chapterId);
    void downloadFailed(long chapterId, const QString& error);
    void queueChanged();

private:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager() = default;

    void processQueue();
    void downloadChapter(DownloadItem& item);

    QQueue<DownloadItem> m_queue;
    QMap<long, DownloadItem> m_activeDownloads;
    QString m_downloadPath;
    bool m_isProcessing = false;
    mutable QMutex m_mutex;
};

#endif // DOWNLOADMANAGER_H
