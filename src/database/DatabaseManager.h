#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <memory>

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

    std::unique_ptr<MangaRepository> m_mangaRepo;
    std::unique_ptr<ChapterRepository> m_chapterRepo;
    std::unique_ptr<HistoryRepository> m_historyRepo;
    std::unique_ptr<CategoryRepository> m_categoryRepo;
};

#endif // DATABASEMANAGER_H
