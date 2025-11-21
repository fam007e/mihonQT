#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

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

    qDebug() << "Database schema initialized successfully.";
    return true;
}
