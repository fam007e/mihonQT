#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class MangaRepository;
class ChapterRepository;
class HistoryRepository;
class CategoryRepository;

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool openDatabase(const QString& path = "mihon.db");
    void closeDatabase();
    QSqlDatabase& database();

    MangaRepository& mangaRepository();
    ChapterRepository& chapterRepository();
    HistoryRepository& historyRepository();
    CategoryRepository& categoryRepository(); // Changed to return a non-const reference

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initializeDatabase();

    QSqlDatabase m_db;

    MangaRepository* m_mangaRepo = nullptr;
    ChapterRepository* m_chapterRepo = nullptr;
    HistoryRepository* m_historyRepo = nullptr;
    CategoryRepository* m_categoryRepo = nullptr;
};

#endif // DATABASEMANAGER_H
