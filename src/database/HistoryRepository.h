#ifndef HISTORYREPOSITORY_H
#define HISTORYREPOSITORY_H

#include <QList>
#include <QString>

class Manga;
class Chapter;

struct HistoryEntry {
    long id;
    long chapterId;
    long mangaId;
    QString mangaTitle;
    QString chapterName;
    QString thumbnailUrl;
    long lastRead;
    long timeRead;
};

class HistoryRepository
{
public:
    explicit HistoryRepository();

    // CRUD operations
    bool upsertHistory(long chapterId, long lastRead, long timeRead = 0);
    bool deleteHistory(long chapterId);
    bool deleteAllHistory();

    // Queries
    QList<HistoryEntry> getRecentHistory(int limit = 50);
    HistoryEntry getHistoryByChapterId(long chapterId);
};

#endif // HISTORYREPOSITORY_H
