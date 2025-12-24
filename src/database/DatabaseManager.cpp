#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include "MangaRepository.h"
#include "ChapterRepository.h"
#include "HistoryRepository.h"
#include "CategoryRepository.h"

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    delete m_mangaRepo;
    delete m_chapterRepo;
    delete m_historyRepo;
    delete m_categoryRepo;
}

bool DatabaseManager::openDatabase(const QString& path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // Set database path in a standard location
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    qDebug() << "Database path resolved to:" << dbPath + "/" + path; // Debug output
    m_db.setDatabaseName(dbPath + "/" + path);

    if (!m_db.open()) {
        qDebug() << "Error: connection with database failed:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully.";
    return initializeDatabase();
}

void DatabaseManager::closeDatabase()
{
    m_db.close();
}

QSqlDatabase& DatabaseManager::database() // Changed to return a non-const reference
{
    return m_db;
}

bool DatabaseManager::initializeDatabase()
{
    QFile schemaFile(":/database/schema.sql");
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open schema.sql file.";
        return false;
    }

    QTextStream in(&schemaFile);
    QString schemaSql = in.readAll();
    schemaFile.close();

    QSqlQuery query;
    QStringList statements = schemaSql.split(';', Qt::SkipEmptyParts);

    for (const QString& statement : statements) {
        if (!statement.trimmed().isEmpty()) {
            if (!query.exec(statement)) {
                qDebug() << "Error executing statement:" << query.lastError().text();
                qDebug() << "Statement:" << statement;
                return false;
            }
        }
    }

    // Ensure 'chapters' table has the new columns for existing databases
    QSqlQuery checkQuery;
    checkQuery.exec("PRAGMA table_info(chapters)");
    bool hasVersion = false;
    bool hasIsSyncing = false;
    while (checkQuery.next()) {
        QString name = checkQuery.value(1).toString();
        if (name == "version") hasVersion = true;
        if (name == "is_syncing") hasIsSyncing = true;
    }

    if (!hasVersion) {
        query.exec("ALTER TABLE chapters ADD COLUMN version INTEGER NOT NULL DEFAULT 0");
    }
    if (!hasIsSyncing) {
        query.exec("ALTER TABLE chapters ADD COLUMN is_syncing INTEGER NOT NULL DEFAULT 0");
    }

    qDebug() << "Database schema initialized successfully.";
    return true;
}

MangaRepository& DatabaseManager::mangaRepository()
{
    if (!m_mangaRepo) m_mangaRepo = new MangaRepository();
    return *m_mangaRepo;
}

ChapterRepository& DatabaseManager::chapterRepository()
{
    if (!m_chapterRepo) m_chapterRepo = new ChapterRepository();
    return *m_chapterRepo;
}

HistoryRepository& DatabaseManager::historyRepository()
{
    if (!m_historyRepo) m_historyRepo = new HistoryRepository();
    return *m_historyRepo;
}

CategoryRepository& DatabaseManager::categoryRepository()
{
    if (!m_categoryRepo) m_categoryRepo = new CategoryRepository();
    return *m_categoryRepo;
}
