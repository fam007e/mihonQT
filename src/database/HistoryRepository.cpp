#include "HistoryRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

HistoryRepository::HistoryRepository()
{
}

bool HistoryRepository::upsertHistory(long chapterId, long lastRead, long timeRead)
{
    QSqlQuery query;
    // Use INSERT OR REPLACE for upsert behavior
    query.prepare("INSERT OR REPLACE INTO history (chapter_id, last_read, time_read) "
                  "VALUES (:chapter_id, :last_read, :time_read)");
    query.bindValue(":chapter_id", static_cast<qlonglong>(chapterId));
    query.bindValue(":last_read", static_cast<qlonglong>(lastRead));
    query.bindValue(":time_read", static_cast<qlonglong>(timeRead));

    if (!query.exec()) {
        qDebug() << "Error upserting history:" << query.lastError().text();
        return false;
    }
    return true;
}

bool HistoryRepository::deleteHistory(long chapterId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM history WHERE chapter_id = :chapter_id");
    query.bindValue(":chapter_id", static_cast<qlonglong>(chapterId));

    if (!query.exec()) {
        qDebug() << "Error deleting history:" << query.lastError().text();
        return false;
    }
    return true;
}

bool HistoryRepository::deleteAllHistory()
{
    QSqlQuery query;
    if (!query.exec("DELETE FROM history")) {
        qDebug() << "Error deleting all history:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<HistoryEntry> HistoryRepository::getRecentHistory(int limit)
{
    QList<HistoryEntry> entries;
    QSqlQuery query;
    query.prepare(
        "SELECT h._id, h.chapter_id, h.last_read, h.time_read, "
        "       c.manga_id, c.name AS chapter_name, "
        "       m.title AS manga_title, m.thumbnail_url "
        "FROM history h "
        "JOIN chapters c ON h.chapter_id = c._id "
        "JOIN mangas m ON c.manga_id = m._id "
        "ORDER BY h.last_read DESC "
        "LIMIT :limit"
    );
    query.bindValue(":limit", limit);

    if (query.exec()) {
        while (query.next()) {
            HistoryEntry entry;
            entry.id = query.value("_id").toLongLong();
            entry.chapterId = query.value("chapter_id").toLongLong();
            entry.mangaId = query.value("manga_id").toLongLong();
            entry.mangaTitle = query.value("manga_title").toString();
            entry.chapterName = query.value("chapter_name").toString();
            entry.thumbnailUrl = query.value("thumbnail_url").toString();
            entry.lastRead = query.value("last_read").toLongLong();
            entry.timeRead = query.value("time_read").toLongLong();
            entries.append(entry);
        }
    } else {
        qDebug() << "Error getting recent history:" << query.lastError().text();
    }
    return entries;
}

HistoryEntry HistoryRepository::getHistoryByChapterId(long chapterId)
{
    HistoryEntry entry;
    entry.id = -1;
    entry.chapterId = -1;
    entry.mangaId = -1;
    entry.lastRead = 0;
    entry.timeRead = 0;

    QSqlQuery query;
    query.prepare(
        "SELECT h._id, h.chapter_id, h.last_read, h.time_read, "
        "       c.manga_id, c.name AS chapter_name, "
        "       m.title AS manga_title, m.thumbnail_url "
        "FROM history h "
        "JOIN chapters c ON h.chapter_id = c._id "
        "JOIN mangas m ON c.manga_id = m._id "
        "WHERE h.chapter_id = :chapter_id"
    );
    query.bindValue(":chapter_id", static_cast<qlonglong>(chapterId));

    if (query.exec() && query.next()) {
        entry.id = query.value("_id").toLongLong();
        entry.chapterId = query.value("chapter_id").toLongLong();
        entry.mangaId = query.value("manga_id").toLongLong();
        entry.mangaTitle = query.value("manga_title").toString();
        entry.chapterName = query.value("chapter_name").toString();
        entry.thumbnailUrl = query.value("thumbnail_url").toString();
        entry.lastRead = query.value("last_read").toLongLong();
        entry.timeRead = query.value("time_read").toLongLong();
    }
    return entry;
}
