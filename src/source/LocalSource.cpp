#include "LocalSource.h"
#include "model/Manga.h"
#include "model/SChapter.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QXmlStreamReader>
#include <QStringConverter>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QCryptographicHash>
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
    manga.setFavorite(false); // Do not favorite by default
    manga.setDateAdded(QDateTime::currentSecsSinceEpoch()); // Or use directory creation/modification time

    // Thumbnail: look for cover image in directory
    QStringList imageFilters = {"cover.*", "folder.*", "poster.*", "*.jpg", "*.png", "*.jpeg"};
    QFileInfoList images = mangaDir.entryInfoList(imageFilters, QDir::Files, QDir::Name);
    if (!images.isEmpty()) {
        manga.setThumbnailUrl(images.first().absoluteFilePath());
    } else {
        // Fallback: try to extract from first CBZ
        QString cachedCover = extractCoverFromArchive(mangaDir);
        manga.setThumbnailUrl(cachedCover);
    }

    return manga;
}

QString LocalSource::extractCoverFromArchive(const QDir& mangaDir) const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers";
    QDir().mkpath(cacheDir);

    QString mangaId = QString(QCryptographicHash::hash(mangaDir.absolutePath().toUtf8(), QCryptographicHash::Md5).toHex());
    QString cachePath = cacheDir + "/" + mangaId + ".jpg";

    if (QFile::exists(cachePath)) {
        return cachePath;
    }

    // Look for archives
    QFileInfoList archives = mangaDir.entryInfoList({"*.cbz", "*.zip"}, QDir::Files, QDir::Name);
    if (archives.isEmpty()) return "";

    QuaZip zip(archives.first().absoluteFilePath());
    if (!zip.open(QuaZip::mdUnzip)) return "";

    // Find first image file
    bool found = false;
    QString firstImage;
    QStringList imageExts = {".jpg", ".jpeg", ".png", ".webp"};

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        QString fileName = zip.getCurrentFileName();
        bool isImage = false;
        for (const QString& ext : imageExts) {
            if (fileName.endsWith(ext, Qt::CaseInsensitive)) {
                isImage = true;
                break;
            }
        }
        if (isImage) {
            firstImage = fileName;
            found = true;
            break;
        }
    }

    if (found) {
        QuaZipFile zipFile(&zip);
        if (zipFile.open(QIODevice::ReadOnly)) {
            QByteArray data = zipFile.readAll();
            QFile outFile(cachePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                outFile.write(data);
                outFile.close();
            }
            zipFile.close();
        }
    }

    zip.close();
    return found ? cachePath : "";
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
    Manga mutableManga = manga;
    QByteArray comicInfoData;
    bool found = false;

    QDir mangaDir(manga.url());
    if (!mangaDir.exists()) return mutableManga;

    // 1. Check for ComicInfo.xml in root directory
    QFile rootComicInfo(mangaDir.filePath("ComicInfo.xml"));
    if (rootComicInfo.exists() && rootComicInfo.open(QIODevice::ReadOnly)) {
        comicInfoData = rootComicInfo.readAll();
        rootComicInfo.close();
        found = true;
    }

    // 2. If not found, look inside first few CBZs
    if (!found) {
        QFileInfoList archives = mangaDir.entryInfoList({"*.cbz", "*.zip"}, QDir::Files, QDir::Name);
        for (int i = 0; i < qMin(3, (int)archives.size()); ++i) {
            QuaZip zip(archives[i].absoluteFilePath());
            if (zip.open(QuaZip::mdUnzip)) {
                if (zip.setCurrentFile("ComicInfo.xml", QuaZip::csInsensitive)) {
                    QuaZipFile zipFile(&zip);
                    if (zipFile.open(QIODevice::ReadOnly)) {
                        comicInfoData = zipFile.readAll();
                        zipFile.close();
                        found = true;
                    }
                }
                zip.close();
            }
            if (found) break;
        }
    }

    if (found) {
        QString xmlString = readComicInfoXml(comicInfoData);
        QXmlStreamReader xml(xmlString);

        while (!xml.atEnd() && !xml.hasError()) {
            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::StartElement) {
                QString name = xml.name().toString();
                if (name == "Series") {
                    QString val = xml.readElementText();
                    if (!val.isEmpty()) mutableManga.setTitle(val);
                }
                else if (name == "Title" && mutableManga.title().isEmpty()) {
                     mutableManga.setTitle(xml.readElementText());
                }
                else if (name == "Summary") mutableManga.setDescription(xml.readElementText());
                else if (name == "Writer") mutableManga.setAuthor(xml.readElementText());
                else if (name == "Penciller") mutableManga.setArtist(xml.readElementText());
                else if (name == "Genre") mutableManga.setGenre(xml.readElementText());
                else if (name == "PublishingStatusTachiyomi") {
                    QString status = xml.readElementText();
                    if (status == "Ongoing") mutableManga.setStatus(MangaStatus::ONGOING);
                    else if (status == "Completed") mutableManga.setStatus(MangaStatus::COMPLETED);
                }
            }
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
        QDir mangaDir(manga.url());
        QFileInfoList entries = mangaDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

        QRegularExpression chapterRegex("(?:^|[\\s_])(?:ch|chapter|ch\\.|v|vol|volume|#)?[\\s_]*(\\d+(?:\\.\\d+)?)", QRegularExpression::CaseInsensitiveOption);

        for (const QFileInfo &entry : entries) {
            SChapter chapter;
            bool isChapter = false;

            if (entry.isDir()) {
                chapter.setUrl(entry.absoluteFilePath());
                isChapter = true;
            } else if (entry.isFile() && (entry.suffix().compare("cbz", Qt::CaseInsensitive) == 0 || entry.suffix().compare("zip", Qt::CaseInsensitive) == 0)) {
                chapter.setUrl(QString("cbz://%1").arg(entry.absoluteFilePath()));
                isChapter = true;
            }

            if (isChapter) {
                chapter.setName(entry.fileName());
                chapter.setDateUpload(entry.lastModified().toSecsSinceEpoch());

                // Try to parse chapter number from filename
                QRegularExpressionMatch match = chapterRegex.match(entry.fileName());
                if (match.hasMatch()) {
                    chapter.setChapterNumber(match.captured(1).toDouble());
                } else {
                    // Fallback to finding ANY number in the string
                    QRegularExpression fallbackRegex("(\\d+(?:\\.\\d+)?)");
                    QRegularExpressionMatch fallbackMatch = fallbackRegex.match(entry.fileName());
                    if (fallbackMatch.hasMatch()) {
                        chapter.setChapterNumber(fallbackMatch.captured(1).toDouble());
                    } else {
                        chapter.setChapterNumber(-1.0); // Unknown
                    }
                }
                chapters.append(chapter);
            }
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
