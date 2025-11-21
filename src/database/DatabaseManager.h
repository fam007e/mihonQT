#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool openDatabase(const QString& path = "mihon.db");
    void closeDatabase();
    QSqlDatabase& database(); // Changed to return a non-const reference

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initializeDatabase();

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
