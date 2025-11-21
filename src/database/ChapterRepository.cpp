#include "ChapterRepository.h"
#include "model/Chapter.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

ChapterRepository::ChapterRepository(QSqlDatabase& db)
    : m_db(db)
{
}

bool ChapterRepository::insertChapter(const Chapter& chapter)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO chapters ("
                  "manga_id, url, name, scanlator, read, bookmark, last_page_read, chapter_number, "
                  "source_order, date_fetch, date_upload, last_modified_at, version, is_syncing"
                  ") VALUES ("
                  ":manga_id, :url, :name, :scanlator, :read, :bookmark, :last_page_read, :chapter_number, "
                  ":source_order, :date_fetch, :date_upload, :last_modified_at, :version, :is_syncing"
                  ")");

    query.bindValue(":manga_id", static_cast<qlonglong>(chapter.mangaId()));
    query.bindValue(":url", chapter.url());
    query.bindValue(":name", chapter.name());
    query.bindValue(":scanlator", chapter.scanlator());
    query.bindValue(":read", chapter.read());
    query.bindValue(":bookmark", chapter.bookmark());
    query.bindValue(":last_page_read", static_cast<qlonglong>(chapter.lastPageRead()));
    query.bindValue(":chapter_number", chapter.chapterNumber());
    query.bindValue(":source_order", static_cast<qlonglong>(chapter.sourceOrder()));
    query.bindValue(":date_fetch", static_cast<qlonglong>(chapter.dateFetch()));
    query.bindValue(":date_upload", static_cast<qlonglong>(chapter.dateUpload()));
    query.bindValue(":last_modified_at", static_cast<qlonglong>(chapter.lastModifiedAt()));
    query.bindValue(":version", static_cast<qlonglong>(chapter.version()));
    query.bindValue(":is_syncing", chapter.isSyncing());

    if (!query.exec()) {
        qDebug() << "Error inserting chapter:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ChapterRepository::updateChapter(const Chapter& chapter)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE chapters SET "
                  "manga_id = :manga_id, url = :url, name = :name, scanlator = :scanlator, "
                  "read = :read, bookmark = :bookmark, last_page_read = :last_page_read, "
                  "chapter_number = :chapter_number, source_order = :source_order, "
                  "date_fetch = :date_fetch, date_upload = :date_upload, "
                  "last_modified_at = :last_modified_at, version = :version, is_syncing = :is_syncing "
                  "WHERE _id = :id");

    query.bindValue(":manga_id", static_cast<qlonglong>(chapter.mangaId()));
    query.bindValue(":url", chapter.url());
    query.bindValue(":name", chapter.name());
    query.bindValue(":scanlator", chapter.scanlator());
    query.bindValue(":read", chapter.read());
    query.bindValue(":bookmark", chapter.bookmark());
    query.bindValue(":last_page_read", static_cast<qlonglong>(chapter.lastPageRead()));
    query.bindValue(":chapter_number", chapter.chapterNumber());
    query.bindValue(":source_order", static_cast<qlonglong>(chapter.sourceOrder()));
    query.bindValue(":date_fetch", static_cast<qlonglong>(chapter.dateFetch()));
    query.bindValue(":date_upload", static_cast<qlonglong>(chapter.dateUpload()));
    query.bindValue(":last_modified_at", static_cast<qlonglong>(chapter.lastModifiedAt()));
    query.bindValue(":version", static_cast<qlonglong>(chapter.version()));
    query.bindValue(":is_syncing", chapter.isSyncing());
    query.bindValue(":id", static_cast<qlonglong>(chapter.id()));

    if (!query.exec()) {
        qDebug() << "Error updating chapter:" << query.lastError().text();
        return false;
    }
    return true;
}

Chapter ChapterRepository::getChapterById(long id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM chapters WHERE _id = :id");
    query.bindValue(":id", static_cast<qlonglong>(id));
    if (query.exec() && query.next()) {
        return chapterFromQuery(query);
    }
    qDebug() << "Error getting chapter by ID:" << query.lastError().text();
    return Chapter(); // Return a default/invalid Chapter object
}

QList<Chapter> ChapterRepository::getChaptersByMangaId(long mangaId)
{
    QList<Chapter> chapters;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM chapters WHERE manga_id = :manga_id ORDER BY chapter_number ASC");
    query.bindValue(":manga_id", static_cast<qlonglong>(mangaId));
    if (query.exec()) {
        while (query.next()) {
            chapters.append(chapterFromQuery(query));
        }
    } else {
        qDebug() << "Error getting chapters by Manga ID:" << query.lastError().text();
    }
    return chapters;
}

bool ChapterRepository::deleteChapter(long id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM chapters WHERE _id = :id");
    query.bindValue(":id", static_cast<qlonglong>(id));
    if (!query.exec()) {
        qDebug() << "Error deleting chapter:" << query.lastError().text();
        return false;
    }
    return true;
}

Chapter ChapterRepository::chapterFromQuery(QSqlQuery& query)
{
    return Chapter(
        query.value("_id").toLongLong(),
        query.value("manga_id").toLongLong(),
        query.value("url").toString(),
        query.value("name").toString(),
        query.value("scanlator").toString(),
        query.value("read").toBool(),
        query.value("bookmark").toBool(),
        query.value("last_page_read").toLongLong(),
        query.value("chapter_number").toDouble(),
        query.value("source_order").toLongLong(),
        query.value("date_fetch").toLongLong(),
        query.value("date_upload").toLongLong(),
        query.value("last_modified_at").toLongLong(),
        query.value("version").toLongLong(),
        query.value("is_syncing").toBool()
    );
}
