#include "MangaRepository.h"
#include "model/Manga.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

MangaRepository::MangaRepository(QSqlDatabase& db)
    : m_db(db)
{
}

bool MangaRepository::insertManga(const Manga& manga)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO mangas ("
                  "source, url, artist, author, description, genre, title, status, thumbnail_url, favorite, "
                  "last_update, next_update, initialized, viewer, chapter_flags, cover_last_modified, "
                  "date_added, update_strategy, calculate_interval, last_modified_at, favorite_modified_at, version, notes, is_syncing"
                  ") VALUES ("
                  ":source, :url, :artist, :author, :description, :genre, :title, :status, :thumbnail_url, :favorite, "
                  ":last_update, :next_update, :initialized, :viewer, :chapter_flags, :cover_last_modified, "
                  ":date_added, :update_strategy, :calculate_interval, :last_modified_at, :favorite_modified_at, :version, :notes, :is_syncing"
                  ")");

    query.bindValue(":source", static_cast<qlonglong>(manga.source()));
    query.bindValue(":url", manga.url());
    query.bindValue(":artist", manga.artist());
    query.bindValue(":author", manga.author());
    query.bindValue(":description", manga.description());
    query.bindValue(":genre", manga.genre());
    query.bindValue(":title", manga.title());
    query.bindValue(":status", manga.status());
    query.bindValue(":thumbnail_url", manga.thumbnailUrl());
    query.bindValue(":favorite", manga.favorite());
    query.bindValue(":last_update", static_cast<qlonglong>(manga.lastUpdate()));
    query.bindValue(":next_update", static_cast<qlonglong>(manga.nextUpdate()));
    query.bindValue(":initialized", manga.initialized());
    query.bindValue(":viewer", static_cast<qlonglong>(manga.viewerFlags())); // Using viewerFlags for viewer
    query.bindValue(":chapter_flags", static_cast<qlonglong>(manga.chapterFlags()));
    query.bindValue(":cover_last_modified", static_cast<qlonglong>(manga.coverLastModified()));
    query.bindValue(":date_added", static_cast<qlonglong>(manga.dateAdded()));
    query.bindValue(":update_strategy", manga.updateStrategy());
    query.bindValue(":calculate_interval", manga.fetchInterval()); // Using fetchInterval for calculate_interval
    query.bindValue(":last_modified_at", static_cast<qlonglong>(manga.lastModifiedAt()));
    query.bindValue(":favorite_modified_at", static_cast<qlonglong>(manga.favoriteModifiedAt()));
    query.bindValue(":version", static_cast<qlonglong>(manga.version()));
    query.bindValue(":notes", manga.notes());
    query.bindValue(":is_syncing", manga.isSyncing());

    if (!query.exec()) {
        qDebug() << "Error inserting manga:" << query.lastError().text();
        return false;
    }
    return true;
}

bool MangaRepository::updateManga(const Manga& manga)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE mangas SET "
                  "source = :source, url = :url, artist = :artist, author = :author, "
                  "description = :description, genre = :genre, title = :title, status = :status, "
                  "thumbnail_url = :thumbnail_url, favorite = :favorite, last_update = :last_update, "
                  "next_update = :next_update, initialized = :initialized, viewer = :viewer, "
                  "chapter_flags = :chapter_flags, cover_last_modified = :cover_last_modified, "
                  "date_added = :date_added, update_strategy = :update_strategy, "
                  "calculate_interval = :calculate_interval, last_modified_at = :last_modified_at, "
                  "favorite_modified_at = :favorite_modified_at, version = :version, notes = :notes, is_syncing = :is_syncing "
                  "WHERE _id = :id");

    query.bindValue(":source", static_cast<qlonglong>(manga.source()));
    query.bindValue(":url", manga.url());
    query.bindValue(":artist", manga.artist());
    query.bindValue(":author", manga.author());
    query.bindValue(":description", manga.description());
    query.bindValue(":genre", manga.genre());
    query.bindValue(":title", manga.title());
    query.bindValue(":status", manga.status());
    query.bindValue(":thumbnail_url", manga.thumbnailUrl());
    query.bindValue(":favorite", manga.favorite());
    query.bindValue(":last_update", static_cast<qlonglong>(manga.lastUpdate()));
    query.bindValue(":next_update", static_cast<qlonglong>(manga.nextUpdate()));
    query.bindValue(":initialized", manga.initialized());
    query.bindValue(":viewer", static_cast<qlonglong>(manga.viewerFlags()));
    query.bindValue(":chapter_flags", static_cast<qlonglong>(manga.chapterFlags()));
    query.bindValue(":cover_last_modified", static_cast<qlonglong>(manga.coverLastModified()));
    query.bindValue(":date_added", static_cast<qlonglong>(manga.dateAdded()));
    query.bindValue(":update_strategy", manga.updateStrategy());
    query.bindValue(":calculate_interval", manga.fetchInterval());
    query.bindValue(":last_modified_at", static_cast<qlonglong>(manga.lastModifiedAt()));
    query.bindValue(":favorite_modified_at", static_cast<qlonglong>(manga.favoriteModifiedAt()));
    query.bindValue(":version", static_cast<qlonglong>(manga.version()));
    query.bindValue(":notes", manga.notes());
    query.bindValue(":is_syncing", manga.isSyncing());
    query.bindValue(":id", static_cast<qlonglong>(manga.id()));

    if (!query.exec()) {
        qDebug() << "Error updating manga:" << query.lastError().text();
        return false;
    }
    return true;
}

Manga MangaRepository::getMangaById(long id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM mangas WHERE _id = :id");
    query.bindValue(":id", static_cast<qlonglong>(id));
    if (query.exec() && query.next()) {
        return mangaFromQuery(query);
    }
    qDebug() << "Error getting manga by ID:" << query.lastError().text();
    return Manga(); // Return a default/invalid Manga object
}

QList<Manga> MangaRepository::getAllManga()
{
    QList<Manga> mangas;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM mangas");
    if (query.exec()) {
        while (query.next()) {
            mangas.append(mangaFromQuery(query));
        }
    } else {
        qDebug() << "Error getting all manga:" << query.lastError().text();
    }
    return mangas;
}

QList<Manga> MangaRepository::getFavorites()
{
    QList<Manga> mangas;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM mangas WHERE favorite = 1");
    if (query.exec()) {
        while (query.next()) {
            mangas.append(mangaFromQuery(query));
        }
    } else {
        qDebug() << "Error getting favorite manga:" << query.lastError().text();
    }
    return mangas;
}

bool MangaRepository::deleteManga(long id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM mangas WHERE _id = :id");
    query.bindValue(":id", static_cast<qlonglong>(id));
    if (!query.exec()) {
        qDebug() << "Error deleting manga:" << query.lastError().text();
        return false;
    }
    return true;
}

Manga MangaRepository::getMangaByUrl(const QString& url, long sourceId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM mangas WHERE url = :url AND source = :source_id");
    query.bindValue(":url", url);
    query.bindValue(":source_id", static_cast<qlonglong>(sourceId));
    if (query.exec() && query.next()) {
        return mangaFromQuery(query);
    }
    // Don't log an error here, as it's a normal case for manga to not exist
    return Manga(); // Return a default/invalid Manga object
}

Manga MangaRepository::mangaFromQuery(QSqlQuery& query)
{
    return Manga(
        query.value("_id").toLongLong(),
        query.value("source").toLongLong(),
        query.value("url").toString(),
        query.value("title").toString(),
        query.value("artist").toString(),
        query.value("author").toString(),
        query.value("description").toString(),
        query.value("genre").toString(),
        query.value("status").toInt(),
        query.value("thumbnail_url").toString(),
        query.value("favorite").toBool(),
        query.value("last_update").toLongLong(),
        query.value("next_update").toLongLong(),
        query.value("calculate_interval").toInt(), // Map calculate_interval to fetchInterval
        query.value("date_added").toLongLong(),
        query.value("viewer").toLongLong(), // Map viewer to viewerFlags
        query.value("chapter_flags").toLongLong(),
        query.value("cover_last_modified").toLongLong(),
        static_cast<UpdateStrategy>(query.value("update_strategy").toInt()),
        query.value("initialized").toBool(),
        query.value("last_modified_at").toLongLong(),
        query.value("favorite_modified_at").toLongLong(),
        query.value("version").toLongLong(),
        query.value("notes").toString(),
        query.value("is_syncing").toBool()
    );
}
