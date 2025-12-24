#include "BackupManager.h"
#include "../database/MangaRepository.h"
#include "../database/ChapterRepository.h"
#include "../database/CategoryRepository.h"
#include "../database/HistoryRepository.h"
#include "../model/Manga.h"
#include "../model/Chapter.h"
#include "../model/Category.h"
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QDebug>

BackupManager& BackupManager::instance()
{
    static BackupManager instance;
    return instance;
}

BackupManager::BackupManager(QObject *parent)
    : QObject(parent)
{
}

QString BackupManager::getDefaultBackupPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    path += "/MihonQT_Backups";
    QDir().mkpath(path);
    return path;
}

bool BackupManager::exportBackup(const QString& filePath)
{
    QJsonObject data = createBackupData();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit exportFailed("Could not open file for writing: " + file.errorString());
        return false;
    }

    QJsonDocument doc(data);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    emit exportComplete(filePath);
    return true;
}

QJsonObject BackupManager::createBackupData()
{
    QJsonObject backup;

    // Metadata
    QJsonObject meta;
    meta["version"] = 1;
    meta["app"] = "MihonQT";
    meta["timestamp"] = QDateTime::currentSecsSinceEpoch();
    backup["metadata"] = meta;

    emit progress(1, 4, "Exporting manga...");
    backup["manga"] = exportManga();

    emit progress(2, 4, "Exporting categories...");
    backup["categories"] = exportCategories();

    emit progress(3, 4, "Exporting chapters...");
    backup["chapters"] = exportChapters();

    emit progress(4, 4, "Exporting history...");
    backup["history"] = exportHistory();

    return backup;
}

QJsonArray BackupManager::exportManga()
{
    QJsonArray arr;
    MangaRepository repo;
    QList<Manga> list = repo.getFavorites();

    for (const Manga& m : list) {
        QJsonObject obj;
        obj["id"] = static_cast<qint64>(m.id());
        obj["source"] = static_cast<qint64>(m.source());
        obj["url"] = m.url();
        obj["title"] = m.title();
        obj["artist"] = m.artist();
        obj["author"] = m.author();
        obj["description"] = m.description();
        obj["genre"] = m.genre();
        obj["status"] = static_cast<int>(m.status());
        obj["thumbnailUrl"] = m.thumbnailUrl();
        obj["favorite"] = m.favorite();
        obj["dateAdded"] = static_cast<qint64>(m.dateAdded());
        arr.append(obj);
    }

    return arr;
}

QJsonArray BackupManager::exportCategories()
{
    QJsonArray arr;
    CategoryRepository repo;
    QList<Category> list = repo.getAllCategories();

    for (const Category& c : list) {
        QJsonObject obj;
        obj["id"] = static_cast<qint64>(c.id());
        obj["name"] = c.name();
        obj["order"] = c.order();
        arr.append(obj);
    }

    return arr;
}

QJsonArray BackupManager::exportChapters()
{
    QJsonArray arr;
    ChapterRepository repo;
    MangaRepository mangaRepo;

    // Export chapters for all favorite manga
    QList<Manga> favorites = mangaRepo.getFavorites();
    for (const Manga& m : favorites) {
        QList<Chapter> chapters = repo.getChaptersByMangaId(m.id());
        for (const Chapter& c : chapters) {
            QJsonObject obj;
            obj["id"] = static_cast<qint64>(c.id());
            obj["mangaId"] = static_cast<qint64>(c.mangaId());
            obj["url"] = c.url();
            obj["name"] = c.name();
            obj["read"] = c.read();
            obj["bookmark"] = c.bookmark();
            obj["lastPageRead"] = static_cast<qint64>(c.lastPageRead());
            obj["chapterNumber"] = c.chapterNumber();
            obj["dateFetch"] = static_cast<qint64>(c.dateFetch());
            arr.append(obj);
        }
    }

    return arr;
}

QJsonArray BackupManager::exportHistory()
{
    QJsonArray arr;
    HistoryRepository repo;
    QList<HistoryEntry> list = repo.getRecentHistory(1000);

    for (const HistoryEntry& h : list) {
        QJsonObject obj;
        obj["chapterId"] = static_cast<qint64>(h.chapterId);
        obj["lastRead"] = static_cast<qint64>(h.lastRead);
        obj["timeRead"] = static_cast<qint64>(h.timeRead);
        arr.append(obj);
    }

    return arr;
}

bool BackupManager::importBackup(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit importFailed("Could not open file: " + file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        emit importFailed("Invalid JSON: " + error.errorString());
        return false;
    }

    return restoreFromBackup(doc.object());
}

bool BackupManager::restoreFromBackup(const QJsonObject& data)
{
    // Check version
    if (!data.contains("metadata")) {
        emit importFailed("Invalid backup format: missing metadata");
        return false;
    }

    int mangaCount = 0;
    int categoryCount = 0;

    emit progress(1, 4, "Importing categories...");
    if (data.contains("categories")) {
        importCategories(data["categories"].toArray());
        categoryCount = data["categories"].toArray().size();
    }

    emit progress(2, 4, "Importing manga...");
    if (data.contains("manga")) {
        importManga(data["manga"].toArray());
        mangaCount = data["manga"].toArray().size();
    }

    emit progress(3, 4, "Importing chapters...");
    if (data.contains("chapters")) {
        importChapters(data["chapters"].toArray());
    }

    emit progress(4, 4, "Importing history...");
    if (data.contains("history")) {
        importHistory(data["history"].toArray());
    }

    emit importComplete(mangaCount, categoryCount);
    return true;
}

bool BackupManager::importManga(const QJsonArray& data)
{
    MangaRepository repo;

    for (const QJsonValue& val : data) {
        QJsonObject obj = val.toObject();

        Manga m(
            obj["id"].toInteger(-1),           // id
            obj["source"].toInteger(0),        // source
            obj["url"].toString(),             // url
            obj["title"].toString(),           // title
            obj["artist"].toString(),          // artist
            obj["author"].toString(),          // author
            obj["description"].toString(),     // description
            obj["genre"].toString(),           // genre
            obj["status"].toInt(),             // status
            obj["thumbnailUrl"].toString(),    // thumbnailUrl
            obj["favorite"].toBool(true),      // favorite
            0,                                 // lastUpdate
            0,                                 // nextUpdate
            0,                                 // fetchInterval
            obj["dateAdded"].toInteger(QDateTime::currentSecsSinceEpoch()), // dateAdded
            0,                                 // viewerFlags
            0,                                 // chapterFlags
            0,                                 // coverLastModified
            UpdateStrategy::ALWAYS_UPDATE,     // updateStrategy
            false,                             // initialized
            0,                                 // lastModifiedAt
            0,                                 // favoriteModifiedAt
            0,                                 // version
            "",                                // notes
            false                              // isSyncing
        );

        // Check if exists, update or insert
        Manga existing = repo.getMangaById(m.id());
        if (existing.id() == -1) {
            repo.insertManga(m);
        } else {
            repo.updateManga(m);
        }
    }

    return true;
}

bool BackupManager::importCategories(const QJsonArray& data)
{
    CategoryRepository repo;

    for (const QJsonValue& val : data) {
        QJsonObject obj = val.toObject();

        Category c(
            obj["id"].toInteger(-1),
            obj["name"].toString(),
            obj["order"].toInt(0),
            0
        );

        // Insert (will handle duplicates)
        repo.insertCategory(c);
    }

    return true;
}

bool BackupManager::importChapters(const QJsonArray& data)
{
    ChapterRepository repo;

    for (const QJsonValue& val : data) {
        QJsonObject obj = val.toObject();

        Chapter c(
            obj["id"].toInteger(-1),
            obj["mangaId"].toInteger(0),
            obj["url"].toString(),
            obj["name"].toString(),
            "",
            obj["read"].toBool(false),
            obj["bookmark"].toBool(false),
            obj["lastPageRead"].toInteger(0),
            obj["chapterNumber"].toDouble(0),
            0,
            obj["dateFetch"].toInteger(0),
            0, 0, 0, false
        );

        // Check if exists
        Chapter existing = repo.getChapterById(c.id());
        if (existing.id() == -1) {
            repo.insertChapter(c);
        } else {
            repo.updateChapter(c);
        }
    }

    return true;
}

bool BackupManager::importHistory(const QJsonArray& data)
{
    HistoryRepository repo;

    for (const QJsonValue& val : data) {
        QJsonObject obj = val.toObject();
        repo.upsertHistory(
            obj["chapterId"].toInteger(),
            obj["lastRead"].toInteger(),
            obj["timeRead"].toInteger(0)
        );
    }

    return true;
}
