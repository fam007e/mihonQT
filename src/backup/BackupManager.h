#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class BackupManager : public QObject
{
    Q_OBJECT

public:
    static BackupManager& instance();

    // Export
    bool exportBackup(const QString& filePath);
    QJsonObject createBackupData();

    // Import
    bool importBackup(const QString& filePath);
    bool restoreFromBackup(const QJsonObject& data);

    // Get backup info
    static QString getDefaultBackupPath();

signals:
    void exportComplete(const QString& path);
    void exportFailed(const QString& error);
    void importComplete(int mangaCount, int categoryCount);
    void importFailed(const QString& error);
    void progress(int current, int total, const QString& message);

private:
    explicit BackupManager(QObject *parent = nullptr);

    QJsonArray exportManga();
    QJsonArray exportCategories();
    QJsonArray exportChapters();
    QJsonArray exportHistory();

    bool importManga(const QJsonArray& data);
    bool importCategories(const QJsonArray& data);
    bool importChapters(const QJsonArray& data);
    bool importHistory(const QJsonArray& data);
};

#endif // BACKUPMANAGER_H
