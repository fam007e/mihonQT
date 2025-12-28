#ifndef LOCALSOURCE_H
#define LOCALSOURCE_H

#include <QString>
#include <QList>
#include <QDir>
#include "SourceBase.h" // Include our new base class
#include <quazip/quazip.h> // For QuaZip
#include <quazip/quazipfile.h> // For QuaZipFile

// Forward declarations for models (SChapter is now defined in SourceBase.h)
class Manga;

class LocalSource : public SourceBase // Inherit from SourceBase
{
    Q_OBJECT // Add Q_OBJECT macro for QObject inheritance

public:
    LocalSource(const QString& baseDirectory = "", QObject *parent = nullptr); // Add QObject parent parameter

    // Override virtual functions from SourceBase
    QString name() const override;
    long id() const override;
    QString lang() const override;
    QString baseUrl() const override;
    bool supportsLatest() const override;

    // Browse related
    QList<Manga> getPopularManga() override;
    QList<Manga> getLatestUpdates() override;
    QList<Manga> getSearchManga(const QString& query) override;

    // Manga details related
    Manga getMangaDetails(const Manga& manga) override;

    // Chapters
    QList<SChapter> getChapterList(const Manga& manga) override;
    QList<QString> getPageList(const Chapter& chapter) override;

    // Helper to set base directory (specific to LocalSource, not in SourceBase)
    void setBaseDirectory(const QString& path);
    QString getBaseDirectory() const;

    static const long ID;
    static const QString NAME;
    static const QString LANG;

private:
    QString m_baseDirectory;

    QList<QDir> getFilesInBaseDirectory() const;
    Manga mangaFromDirectory(const QDir& mangaDir) const;
    QString extractCoverFromArchive(const QDir& mangaDir) const;
};

#endif // LOCALSOURCE_H
