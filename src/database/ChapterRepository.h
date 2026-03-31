#ifndef CHAPTERREPOSITORY_H
#define CHAPTERREPOSITORY_H

#include <QSqlDatabase>
#include <QString>
#include <QList>

// Forward declaration of Chapter model
class Chapter;

class ChapterRepository
{
public:
    explicit ChapterRepository(); // Removed QSqlDatabase argument

    // CRUD operations
    bool insertChapter(const Chapter& chapter);
    bool updateChapter(const Chapter& chapter);
    Chapter getChapterById(long id);
    QList<Chapter> getChaptersByMangaId(long mangaId);
    int getUnreadCountByMangaId(long mangaId); // NEW
    int getTotalChapterCount();
    int getReadChapterCount();
    bool deleteChapter(long id);

private:
    // QSqlDatabase m_db; // Removed member // Store by value

    Chapter chapterFromQuery(QSqlQuery& query);
};

#endif // CHAPTERREPOSITORY_H
