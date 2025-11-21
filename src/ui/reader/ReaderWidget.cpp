#include "ReaderWidget.h"
#include "../../model/Manga.h"
#include "../../model/Chapter.h"
#include "../../source/LocalSource.h"
#include "../../database/ChapterRepository.h"
#include "../../database/MangaRepository.h" // Include MangaRepository.h
#include "../../database/DatabaseManager.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QPixmap>
#include <QImageReader>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QThread> // For background loading
#include <QtConcurrent/QtConcurrent> // For asynchronous tasks
#include <quazip/quazipfile.h> // For QuaZipFile for CBZ reading
#include <QKeyEvent>

ReaderWidget::ReaderWidget(QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_scrollArea(new QScrollArea(this)),
      m_contentWidget(new QWidget(this)),
      m_contentLayout(new QVBoxLayout(m_contentWidget)),
      m_pagedLabel(new QLabel(this)),
      m_currentPageIndex(0),
      m_readingMode(Webtoon),
      m_localSource(new LocalSource()),
      m_chapterRepo(new ChapterRepository(DatabaseManager::instance().database())),
      m_mangaRepo(new MangaRepository(DatabaseManager::instance().database()))
{
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Setup Webtoon View
    m_contentWidget->setLayout(m_contentLayout);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    m_contentLayout->setAlignment(Qt::AlignTop);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Setup Paged View
    m_pagedLabel->setAlignment(Qt::AlignCenter);
    m_pagedLabel->hide(); // Hidden by default

    m_mainLayout->addWidget(m_scrollArea);
    m_mainLayout->addWidget(m_pagedLabel);
    
    setLayout(m_mainLayout);

    connect(this, &ReaderWidget::pageLoaded, this, &ReaderWidget::displayPage);
    
    setFocusPolicy(Qt::StrongFocus); // Enable focus for key events
}

ReaderWidget::~ReaderWidget()
{
    delete m_localSource;
    delete m_chapterRepo;
    delete m_mangaRepo;
}

void ReaderWidget::loadManga(const Manga& manga)
{
    m_currentManga = manga;
    m_localSource->setBaseDirectory(m_currentManga.url()); // Manga.url is the path for local source
    m_chapters = m_chapterRepo->getChaptersByMangaId(m_currentManga.id());
    if (!m_chapters.isEmpty()) {
        loadAndDisplayPages(m_chapters.first()); // Load the first chapter for MVP
    } else {
        qDebug() << "No chapters found for manga:" << m_currentManga.title();
        // Display an empty state or error message
    }
}

void ReaderWidget::loadChapter(long mangaId, long chapterId)
{
    // For MVP, we'll assume mangaId is known and load the specific chapter
    // In a real app, you'd load the manga first, then its chapters
    m_currentManga = m_mangaRepo->getMangaById(mangaId);
    if (m_currentManga.id() == -1) {
        qDebug() << "ReaderWidget: Manga not found for ID:" << mangaId;
        return;
    }

    m_localSource->setBaseDirectory(m_currentManga.url());
    m_chapters = m_chapterRepo->getChaptersByMangaId(mangaId);
    
    Chapter chapterToLoad;
    if (chapterId == -1 && !m_chapters.isEmpty()) {
        chapterToLoad = m_chapters.first(); // Load the first chapter if ID is -1
    } else {
        for (const Chapter& chap : m_chapters) {
            if (chap.id() == chapterId) {
                chapterToLoad = chap;
                break;
            }
        }
    }

    if (chapterToLoad.id() != -1) {
        loadAndDisplayPages(chapterToLoad);
    } else {
        qDebug() << "ReaderWidget: Chapter not found for ID:" << chapterId << "in manga:" << mangaId;
        // Display an error or go back to manga list view
        // For now, emit navigateBack as a fallback
        emit navigateBack();
    }
}


void ReaderWidget::clearChapterContent()
{
    // Clear Webtoon content
    while (QLayoutItem* item = m_contentLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    // Clear Paged content
    m_loadedPages.clear();
    m_pagedLabel->clear();
    m_currentPageIndex = 0;
}

void ReaderWidget::loadAndDisplayPages(const Chapter& chapter)
{
    clearChapterContent();
    qDebug() << "ReaderWidget: Loading pages for chapter:" << chapter.name() << "URL:" << chapter.url();

    QList<QString> pageUrls; // List of URLs for pages within the chapter
    QString chapterUrl = chapter.url();

    if (chapterUrl.startsWith("cbz://")) {
        chapterUrl.remove(0, 6); // Remove "cbz://" prefix to get archive path
        QString archivePath = chapterUrl;
        
        QuaZip zip(archivePath);
        if (!zip.open(QuaZip::mdUnzip)) {
            qWarning() << "ReaderWidget: Could not open CBZ archive for chapter:" << archivePath;
            return;
        }
        
        QStringList allFiles = zip.getFileNameList();
        qDebug() << "ReaderWidget: Found" << allFiles.size() << "total entries in CBZ.";
        for (const QString& fileName : allFiles) {
            QFileInfo fileInfo(fileName);
            QString suffix = fileInfo.suffix();
            if (suffix.compare("jpg", Qt::CaseInsensitive) == 0 ||
                suffix.compare("png", Qt::CaseInsensitive) == 0 ||
                suffix.compare("jpeg", Qt::CaseInsensitive) == 0 ||
                suffix.compare("gif", Qt::CaseInsensitive) == 0 ||
                suffix.compare("webp", Qt::CaseInsensitive) == 0 ||
                suffix.compare("bmp", Qt::CaseInsensitive) == 0) {
                pageUrls.append(QString("cbz://%1/%2").arg(archivePath, fileName));
            }
        }
        zip.close();
        std::sort(pageUrls.begin(), pageUrls.end());
    } else {
        // Chapter URL is a directory path
        QDir chapterDir(chapterUrl);
        if (!chapterDir.exists() || !chapterDir.isReadable()) {
            qWarning() << "Chapter directory does not exist or is not readable:" << chapterUrl;
            return;
        }
        QFileInfoList entries = chapterDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries) {
            if (entry.isFile() && (entry.suffix().compare("jpg", Qt::CaseInsensitive) == 0 ||
                                   entry.suffix().compare("png", Qt::CaseInsensitive) == 0 ||
                                   entry.suffix().compare("jpeg", Qt::CaseInsensitive) == 0)) {
                pageUrls.append(entry.absoluteFilePath());
            }
        }
        std::sort(pageUrls.begin(), pageUrls.end());
    }

    if (pageUrls.isEmpty()) {
        qDebug() << "ReaderWidget: No image files found for chapter:" << chapter.name() << "URL:" << chapter.url();
        QLabel* errorLabel = new QLabel("No pages found for this chapter.", m_contentWidget);
        errorLabel->setAlignment(Qt::AlignCenter);
        m_contentLayout->addWidget(errorLabel);
        return;
    }

    // Load and display images asynchronously
    int pageIndex = 0;
    for (const QString& path : pageUrls) {
        QThreadPool::globalInstance()->start([this, path, pageIndex]() {
            QImage image;
            if (path.startsWith("cbz://")) {
                // ... (CBZ loading logic same as before) ...
                QString url = path;
                url.remove(0, 6);
                int cbzIndex = url.indexOf(".cbz/", 0, Qt::CaseInsensitive);
                if (cbzIndex != -1) {
                    int archivePathLength = cbzIndex + 4;
                    QString archivePath = url.left(archivePathLength);
                    QString imagePathInArchive = url.mid(archivePathLength + 1);
                    QuaZip zip(archivePath);
                    if (zip.open(QuaZip::mdUnzip)) {
                        if (zip.setCurrentFile(imagePathInArchive)) {
                            QuaZipFile file(&zip);
                            if (file.open(QIODevice::ReadOnly)) {
                                image.loadFromData(file.readAll());
                                file.close();
                            }
                        }
                        zip.close();
                    }
                }
            } else {
                QImageReader reader(path);
                reader.setAutoTransform(true);
                image = reader.read();
            }

            if (!image.isNull()) {
                emit pageLoaded(image, pageIndex);
            }
        });
        pageIndex++;
    }
    
    // Resize loaded pages vector
    m_loadedPages.resize(pageUrls.size());
}

void ReaderWidget::displayPage(const QImage& image, int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < m_loadedPages.size()) {
        m_loadedPages[pageIndex] = image;
    }

    if (m_readingMode == Webtoon) {
        QLabel* label = new QLabel(m_contentWidget);
        QPixmap pixmap = QPixmap::fromImage(image);
        
        int targetWidth = m_scrollArea->viewport()->width();
        if (targetWidth <= 0) targetWidth = this->width();
        
        if (targetWidth > 0 && pixmap.width() > targetWidth) {
            pixmap = pixmap.scaledToWidth(targetWidth, Qt::SmoothTransformation);
        }
        
        label->setPixmap(pixmap);
        label->setScaledContents(false);
        label->setAlignment(Qt::AlignCenter);
        
        // Insert in correct order (simple append for now, assuming sequential loading or re-sorting)
        // For proper ordering with async loading, we should insert at specific index or sort layout
        // For MVP, we just append. A better approach is to pre-create labels.
        m_contentLayout->addWidget(label); 
    } else {
        // For Paged mode, update current view if this is the current page
        if (pageIndex == m_currentPageIndex) {
            updateView();
        }
    }
}

void ReaderWidget::setReadingMode(ReadingMode mode)
{
    m_readingMode = mode;
    if (m_readingMode == Webtoon) {
        m_scrollArea->show();
        m_pagedLabel->hide();
        // Re-populate webtoon layout from m_loadedPages if needed
    } else {
        m_scrollArea->hide();
        m_pagedLabel->show();
        updateView();
    }
}

void ReaderWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_D:
        if (m_readingMode == RightToLeft) previousPage();
        else nextPage();
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        if (m_readingMode == RightToLeft) nextPage();
        else previousPage();
        break;
    case Qt::Key_Space:
        nextPage();
        break;
    case Qt::Key_Backspace:
        emit navigateBack();
        break;
    case Qt::Key_Escape:
        emit navigateBack();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void ReaderWidget::nextPage()
{
    if (m_readingMode == Webtoon) {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->value() + m_scrollArea->viewport()->height() / 2
        );
    } else {
        if (m_currentPageIndex < m_loadedPages.size() - 1) {
            m_currentPageIndex++;
            updateView();
        }
    }
}

void ReaderWidget::previousPage()
{
    if (m_readingMode == Webtoon) {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->value() - m_scrollArea->viewport()->height() / 2
        );
    } else {
        if (m_currentPageIndex > 0) {
            m_currentPageIndex--;
            updateView();
        }
    }
}

void ReaderWidget::updateView()
{
    if (m_readingMode != Webtoon && m_currentPageIndex >= 0 && m_currentPageIndex < m_loadedPages.size()) {
        QImage img = m_loadedPages[m_currentPageIndex];
        if (!img.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(img);
            int w = this->width();
            int h = this->height();
            m_pagedLabel->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}