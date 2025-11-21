#include "LocalSource.h"
#include "model/Manga.h"
#include "model/SChapter.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QXmlStreamReader>
#include <QStringConverter>
#include <QRegularExpression>
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

const long LocalSource::ID = 0L;
const QString LocalSource::NAME = "Local source";
const QString LocalSource::LANG = "other";

LocalSource::LocalSource(const QString& baseDirectory, QObject *parent)
    : SourceBase(parent) // Pass parent to base class constructor
    , m_baseDirectory(baseDirectory)
{
}

QString LocalSource::name() const
{
    return NAME;
}

long LocalSource::id() const
{
    return ID;
}

QString LocalSource::lang() const
{
    return LANG;
}

bool LocalSource::supportsLatest() const
{
    return true; // Local files can be sorted by last modified date
}

void LocalSource::setBaseDirectory(const QString& path)
{
    m_baseDirectory = path;
}

QString LocalSource::getBaseDirectory() const
{
    return m_baseDirectory;
}

QList<QDir> LocalSource::getFilesInBaseDirectory() const
{
    QList<QDir> mangaDirs;
    if (m_baseDirectory.isEmpty()) {
        return mangaDirs;
    }

    QDir baseDir(m_baseDirectory);
    if (!baseDir.exists() || !baseDir.isReadable()) {
        qWarning() << "Base directory does not exist or is not readable:" << m_baseDirectory;
        return mangaDirs;
    }

    QFileInfoList entries = baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
    for (const QFileInfo &entry : entries) {
        if (entry.isDir() && !entry.fileName().startsWith('.')) { // Filter out hidden directories
            mangaDirs.append(QDir(entry.absoluteFilePath()));
        }
    }
    return mangaDirs;
}

Manga LocalSource::mangaFromDirectory(const QDir& mangaDir) const
{
    // Simplified for MVP: create a Manga object from directory name
    // No advanced metadata parsing yet (ComicInfo.xml, etc.)
    Manga manga;
    manga.setTitle(mangaDir.dirName());
    manga.setUrl(mangaDir.absolutePath()); // Using absolute path as URL for local source
    manga.setSource(ID);
    manga.setInitialized(true);
    manga.setFavorite(true); // Assume local manga are always favorited
    manga.setDateAdded(QDateTime::currentSecsSinceEpoch()); // Or use directory creation/modification time

    // Dummy thumbnail_url for now
    // In a real implementation, you'd scan the directory for a cover image
    manga.setThumbnailUrl(""); 

    return manga;
}

QList<Manga> LocalSource::getPopularManga()
{
    return getSearchManga(""); // For MVP, popular is just all manga
}

QList<Manga> LocalSource::getLatestUpdates()
{
    QList<Manga> mangas;
    QList<QDir> mangaDirs = getFilesInBaseDirectory();

    // Sort by last modified date (descending for latest)
    std::sort(mangaDirs.begin(), mangaDirs.end(), [](const QDir& a, const QDir& b) {
        return QFileInfo(a.absolutePath()).lastModified() > QFileInfo(b.absolutePath()).lastModified();
    });

    for (const QDir& dir : mangaDirs) {
        mangas.append(mangaFromDirectory(dir));
    }
    return mangas;
}

QList<Manga> LocalSource::getSearchManga(const QString& query)
{
    QList<Manga> mangas;
    QList<QDir> mangaDirs = getFilesInBaseDirectory();

    for (const QDir& dir : mangaDirs) {
        if (query.isEmpty() || dir.dirName().contains(query, Qt::CaseInsensitive)) {
            mangas.append(mangaFromDirectory(dir));
        }
    }
    return mangas;
}

// Helper function to read ComicInfo.xml with various encodings
QString readComicInfoXml(const QByteArray &data)
{
    // Try UTF-8 first (most common)
    QString utf8String = QString::fromUtf8(data);
    if (utf8String.contains("ComicInfo", Qt::CaseInsensitive)) {
        return utf8String;
    }

    // Fallback: just return Latin1 if UTF-8 failed to look like XML
    return QString::fromLatin1(data);
}

Manga LocalSource::getMangaDetails(const Manga& manga)
{
    Manga mutableManga = manga; // Make a mutable copy
    QByteArray comicInfoData;
    bool found = false;

    QFileInfo mangaPathInfo(manga.url());

    if (mangaPathInfo.isDir()) {
        QFile comicInfoFile(manga.url() + "/ComicInfo.xml");
        if (comicInfoFile.exists() && comicInfoFile.open(QIODevice::ReadOnly)) {
            comicInfoData = comicInfoFile.readAll();
            comicInfoFile.close();
            found = true;
        }
    } else if (manga.url().startsWith("cbz://")) {
        QString archivePath = manga.url();
        archivePath.remove(0, 6);
        QuaZip zip(archivePath);
        if (zip.open(QuaZip::mdUnzip)) {
            if (zip.setCurrentFile("ComicInfo.xml", QuaZip::csInsensitive)) {
                QuaZipFile comicInfoFile(&zip);
                if (comicInfoFile.open(QIODevice::ReadOnly)) {
                    comicInfoData = comicInfoFile.readAll();
                    comicInfoFile.close();
                    found = true;
                }
            }
            zip.close();
        }
    }

    if (found) {
        QString xmlString = readComicInfoXml(comicInfoData);
        QXmlStreamReader xml(xmlString);

        while (!xml.atEnd() && !xml.hasError()) {
            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::StartElement) {
                if (xml.name() == QLatin1String("Title"))
                    mutableManga.setTitle(xml.readElementText());
                else if (xml.name() == QLatin1String("Series"))
                    mutableManga.setTitle(xml.readElementText()); // Often series is the main title
                else if (xml.name() == QLatin1String("Summary"))
                    mutableManga.setDescription(xml.readElementText());
                else if (xml.name() == QLatin1String("Writer"))
                    mutableManga.setAuthor(xml.readElementText());
                else if (xml.name() == QLatin1String("Penciller"))
                    mutableManga.setArtist(xml.readElementText());
                else if (xml.name() == QLatin1String("Genre"))
                    mutableManga.setGenre(xml.readElementText());
                else if (xml.name() == QLatin1String("PublishingStatusTachiyomi"))
                {
                    QString status = xml.readElementText();
                    if (status == "Ongoing") mutableManga.setStatus(MangaStatus::ONGOING);
                    else if (status == "Completed") mutableManga.setStatus(MangaStatus::COMPLETED);
                    // Add other status mappings as needed
                }
            }
        }
        if (xml.hasError()) {
            qWarning() << "XML parsing error:" << xml.errorString();
        }
    }

    return mutableManga;
}

QList<SChapter> LocalSource::getChapterList(const Manga& manga)
{
    qDebug() << "LocalSource::getChapterList called for manga:" << manga.title() << "URL:" << manga.url();

    QList<SChapter> chapters;
    QFileInfo mangaPathInfo(manga.url());

    if (!mangaPathInfo.exists() || !mangaPathInfo.isReadable()) {
        qWarning() << "Manga path does not exist or is not readable:" << manga.url();
        return chapters;
    }

    if (mangaPathInfo.isDir()) {
        qDebug() << "Manga URL is a directory:" << manga.url();
        QDir mangaDir(manga.url());
        QFileInfoList entries = mangaDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        qDebug() << "Found" << entries.size() << "entries in manga directory.";
        
        double chapterNumber = 1.0;
        for (const QFileInfo &entry : entries) {
            qDebug() << "Processing entry:" << entry.fileName() << "Is Dir:" << entry.isDir() << "Is File:" << entry.isFile() << "Suffix:" << entry.suffix();
            if (entry.isDir()) {
                // Treat sub-directory as a chapter
                SChapter chapter;
                chapter.setUrl(entry.absoluteFilePath()); // URL is path to chapter folder
                chapter.setName(entry.fileName());
                chapter.setDateUpload(entry.lastModified().toSecsSinceEpoch());
                chapter.setChapterNumber(chapterNumber++);
                chapters.append(chapter);
                qDebug() << "Added directory chapter:" << chapter.name() << "URL:" << chapter.url();
            } else if (entry.isFile() && entry.suffix().compare("cbz", Qt::CaseInsensitive) == 0) {
                // Treat CBZ file as a chapter
                SChapter chapter;
                chapter.setUrl(QString("cbz://%1").arg(entry.absoluteFilePath())); // URL is path to CBZ file
                chapter.setName(entry.fileName());
                chapter.setDateUpload(entry.lastModified().toSecsSinceEpoch());
                chapter.setChapterNumber(chapterNumber++);
                chapters.append(chapter);
                qDebug() << "Added CBZ file chapter:" << chapter.name() << "URL:" << chapter.url();
            }
            // Ignore other loose files for now
        }
    } else if (mangaPathInfo.isFile() && mangaPathInfo.suffix().compare("cbz", Qt::CaseInsensitive) == 0) {
        qDebug() << "Manga URL is a CBZ file:" << manga.url();
        // Manga itself is a single CBZ file, so it represents one "chapter"
        // The reader then needs to extract pages from this single chapter.
        SChapter chapter;
        chapter.setUrl(QString("cbz://%1").arg(manga.url())); // URL is path to CBZ file
        chapter.setName(mangaPathInfo.fileName());
        chapter.setDateUpload(mangaPathInfo.lastModified().toSecsSinceEpoch());
        chapter.setChapterNumber(1.0); // Only one chapter
        chapters.append(chapter);
        qDebug() << "Added single CBZ manga as chapter:" << chapter.name() << "URL:" << chapter.url();

    } else {
        qWarning() << "Unsupported manga file type or format for direct manga file:" << manga.url();
    }
    
    // Sort chapters by chapterNumber or natural sort of name
    std::sort(chapters.begin(), chapters.end(), [](const SChapter& a, const SChapter& b) {
        // Handle potential non-numeric chapter names for sorting, or use default chapterNumber
        return a.chapterNumber() < b.chapterNumber();
    });

    qDebug() << "LocalSource::getChapterList returning" << chapters.size() << "chapters.";
    return chapters;
}
