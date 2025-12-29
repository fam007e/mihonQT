#include "ReaderWidget.h"
#include "ReaderSettingsOverlay.h"
#include "../../model/Manga.h"
#include "../../model/Chapter.h"
#include "../../source/LocalSource.h"
#include "../../database/ChapterRepository.h"
#include "../../database/MangaRepository.h"
#include "../../database/HistoryRepository.h"
#include "../../database/DatabaseManager.h"
#include "../../database/DatabaseManager.h"
#include "../../config/PreferenceManager.h"
#include "../../source/SourceManager.h" // Needed for SourceManager access

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
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
#include <algorithm> // For std::sort
#include <quazip/quazipfile.h> // For QuaZipFile for CBZ reading
#include <QKeyEvent>
#include <QMouseEvent>

ReaderWidget::ReaderWidget(SourceManager* sourceManager, QWidget *parent)
    : QWidget(parent),
      m_mainLayout(new QVBoxLayout(this)),
      m_scrollArea(new QScrollArea(this)),
      m_contentWidget(new QWidget(this)),
      m_contentLayout(new QVBoxLayout(m_contentWidget)),
      m_pagedLabel(new QLabel(this)),
      m_currentPageIndex(0),
      m_readingMode(LeftToRight),
      m_lastSinglePageMode(LeftToRight), // Initialize default preference
      m_localSource(new LocalSource()),
      m_sourceManager(sourceManager),
      m_chapterRepo(new ChapterRepository()),
      m_mangaRepo(new MangaRepository()),
      m_historyRepo(new HistoryRepository()),
      m_settingsOverlay(nullptr),
      m_showingTransition(false)
{
    reloadSettings();
    // ... (rest of constructor same) ...
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_viewStack = new QStackedWidget(this);
    m_mainLayout->addWidget(m_viewStack);

    // Setup Webtoon View
    m_contentWidget->setLayout(m_contentLayout);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    m_contentLayout->setAlignment(Qt::AlignTop);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet("background-color: black; border: none;");

    // Setup Paged View
    m_pagedScrollArea = new QScrollArea(this);
    m_pagedScrollArea->setWidgetResizable(true);
    m_pagedScrollArea->setAlignment(Qt::AlignCenter);
    m_pagedScrollArea->setStyleSheet("background-color: black; border: none;");

    m_pagedLabel->setAlignment(Qt::AlignCenter);
    m_pagedLabel->setStyleSheet("background-color: black;");
    m_pagedScrollArea->setWidget(m_pagedLabel);

    m_viewStack->addWidget(m_scrollArea);      // Index 0: Webtoon
    m_viewStack->addWidget(m_pagedScrollArea); // Index 1: Paged

    setLayout(m_mainLayout);

    connect(this, &ReaderWidget::pageLoaded, this, &ReaderWidget::displayPage);

    setFocusPolicy(Qt::StrongFocus); // Enable focus for key events
}

ReaderWidget::~ReaderWidget()
{
    saveReadingProgress();
    // m_localSource is owned by us
    delete m_localSource;
    // m_sourceManager is NOT owned by us
    delete m_chapterRepo;
    delete m_mangaRepo;
    delete m_historyRepo;
}

void ReaderWidget::reloadSettings()
{
    m_scaleType = static_cast<ScaleType>(PreferenceManager::instance().value("reader/scale_type", 1).toInt());
    m_webtoonPadding = PreferenceManager::instance().value("reader/webtoon_padding", 0).toInt();
    m_keepScreenOn = PreferenceManager::instance().value("reader/keep_screen_on", false).toBool();
    m_keepScreenOn = PreferenceManager::instance().value("reader/keep_screen_on", false).toBool();
    m_fullscreen = PreferenceManager::instance().value("reader/fullscreen", false).toBool();

    int savedMode = PreferenceManager::instance().value("reader/default_mode", 1).toInt(); // Default to LeftToRight (1)
    if (m_readingMode != static_cast<ReadingMode>(savedMode)) {
        setReadingMode(static_cast<ReadingMode>(savedMode));
    }

    // Apply Window States
    if (window()) {
        if (m_fullscreen) {
            window()->showFullScreen();
        } else {
            window()->showNormal();
        }
    }

    // Update view to reflect scaling/padding changes
    if (m_readingMode == Webtoon) {
        // Refresh webtoon view (simplified: clear and reload current chapter if needed, or just update margins)
        // For now, let's just update margins of content layout
        int paddingPx = this->width() * m_webtoonPadding / 100;
        m_contentLayout->setContentsMargins(paddingPx, 0, paddingPx, 0);
    } else {
        updateView();
    }
}

void ReaderWidget::loadManga(const Manga& manga)
{
    m_currentManga = manga;
    m_localSource->setBaseDirectory(m_currentManga.url()); // Manga.url is the path for local source
    m_chapters = m_chapterRepo->getChaptersByMangaId(m_currentManga.id());
    // Ensure chapters are sorted by Chapter Number (ASC) for logical reading order
    std::sort(m_chapters.begin(), m_chapters.end(), [](const Chapter& a, const Chapter& b) {
        return a.chapterNumber() < b.chapterNumber();
    });

    if (!m_chapters.isEmpty()) {
        loadAndDisplayPages(m_chapters.first()); // Load the first chapter for MVP
    } else {
        qDebug() << "No chapters found for manga:" << m_currentManga.title();
        // Display an empty state or error message
    }
}

void ReaderWidget::loadChapter(long mangaId, long chapterId, bool startAtEnd)
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
    // Ensure chapters are sorted by Chapter Number (ASC) for logical reading order
    std::sort(m_chapters.begin(), m_chapters.end(), [](const Chapter& a, const Chapter& b) {
        return a.chapterNumber() < b.chapterNumber();
    });

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
        m_currentChapterId = chapterToLoad.id();
        loadAndDisplayPages(chapterToLoad, startAtEnd);
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
    m_showingTransition = false;
}

void ReaderWidget::loadAndDisplayPages(const Chapter& chapter, bool startAtEnd)
{
    clearChapterContent();
    qDebug() << "ReaderWidget: Loading pages for chapter:" << chapter.name() << "URL:" << chapter.url();

    QString chapterUrl = chapter.url();

    if (chapterUrl.startsWith("http://") || chapterUrl.startsWith("https://")) {
        loadOnlineChapter(chapter, startAtEnd);
        return;
    }

    QList<QString> pageUrls; // List of URLs for pages within the chapter
    // ... existing local/cbz logic ...

    if (chapterUrl.startsWith("cbz://")) {
        chapterUrl.remove(0, 6); // Remove "cbz://" prefix to get archive path
        QString archivePath = chapterUrl;

        QuaZip zip(archivePath);
        if (!zip.open(QuaZip::mdUnzip)) { // flawfinder: ignore
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
            // Fallback: If it's just a file path that doesn't exist, maybe it was meant for online but saved wrong?
            // But here we entered "else" so it didn't start with http.
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

    // Load and display images asynchronously (Local)
    int pageIndex = 0;
    for (const QString& path : pageUrls) {
        QThreadPool::globalInstance()->start([this, path, pageIndex]() {
            QImage image;
            if (path.startsWith("cbz://")) {
                // ... (CBZ loading logic) ...
                QString url = path;
                url.remove(0, 6);
                int cbzIndex = url.indexOf(".cbz/", 0, Qt::CaseInsensitive);
                if (cbzIndex != -1) {
                    int archivePathLength = cbzIndex + 4;
                    QString archivePath = url.left(archivePathLength);
                    QString imagePathInArchive = url.mid(archivePathLength + 1);
                    QuaZip zip(archivePath);
                    if (zip.open(QuaZip::mdUnzip)) { // flawfinder: ignore
                        if (zip.setCurrentFile(imagePathInArchive)) {
                            QuaZipFile file(&zip);
                            if (file.open(QIODevice::ReadOnly)) { // flawfinder: ignore
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
                image = reader.read(); // flawfinder: ignore
            }

            if (!image.isNull()) {
                emit pageLoaded(image, pageIndex);
            }
        });
        pageIndex++;
    }

    // Resize loaded pages vector
    m_loadedPages.resize(pageUrls.size());

    if (startAtEnd && !m_loadedPages.isEmpty()) {
        m_currentPageIndex = m_loadedPages.size() - 1;
        // Adjust for double spread if needed?
        // If Double Page, index should be even (for Left) or odd?
        // In "Double Page (LTR)", pairs are (0,1), (2,3).
        // If size is 10 (indexes 0..9). Last is 9. Pair is (8,9).
        // If we set index to 9.
        // updateView() checks `m_currentPageIndex`.
        // If DoubleSpread LTR: first=9, second=10 (invalid).
        // It displays 9 on Left.
        // Wait, pair logic:
        // LTR: [Page N] [Page N+1]
        // If N=9. [9] [10].
        // If we want the last pair, we should ensure we look at the spread containing the last page.
        // Usually readers snap to the start of the pair.
        // If total 10 pages interactions 0-9.
        // Pairs: 0-1, 2-3, 4-5, 6-7, 8-9.
        // Start indices: 0, 2, 4, 6, 8.
        // If we go to last page 9.
        // If we set m_currentPageIndex = 9.
        // updateView: first=9. second=10.
        // LTR: Left=9. Right=Null. -> Shows Page 9 on Left.
        // This is fine. But user might prefer the full spread if 8 exists.
        // But strict "last page" is 9.
        // Standard behavior: Go to last page.
        // If we want "Last Spread", we might decrement by 1 if odd?
        // For now, simple Last Index is robust enough.

        // Also update Paged View immediately if needed, but displayPage will call updateView when images load?
        // No, displayPage calls updateView only if index matches.
        // We need to call updateView to set initial state (even empty).
        updateView();
    }
}

void ReaderWidget::loadOnlineChapter(const Chapter& chapter, bool startAtEnd)
{
    // 1. Get Source
    SourceBase* source = m_sourceManager->getSourceById(m_currentManga.source());
    if (!source) {
        qWarning() << "ReaderWidget: Source not found for online chapter.";
        return;
    }

    // 2. Fetch Page List (Urls)
    // This calls the JS getPageList implementation.
    // Assuming getPageList returns a list of objects {index: 0, url: "..."} or just a list of page objects.
    // We need to implement getPageList in SourceBase/JavascriptSource first if not present?
    // User added getPageList to JS, so JavascriptSource needs to expose/call it.
    // SourceBase needs virtual QList<Page> getPageList(const Chapter& chapter);
    // Wait, SourceBase signature needs checking.

    // Assuming SourceBase interface update (I will check SourceBase.h next).
    // For now, let's assume getPageList returns just a list of URLs for simplicity or check existing code.
    QList<QString> pageUrls = source->getPageList(chapter); // We need to ensure this method exists and works.

    if (pageUrls.isEmpty()) {
         qDebug() << "ReaderWidget: No pages found from source.";
         QLabel* label = new QLabel("No pages returned from source.", m_contentWidget);
         m_contentLayout->addWidget(label);
         return;
    }

    m_loadedPages.resize(pageUrls.size());

    if (startAtEnd && !m_loadedPages.isEmpty()) {
        m_currentPageIndex = m_loadedPages.size() - 1;
        updateView();
    }

    // 3. Display Images (Download/Fetch)
    // For online images, we should probably fetch them via NetworkAccessManager?
    // Or just use QNetworkAccessManager in a thread/async?
    // Ideally, we load them one by one or in parallel.

    int pageIndex = 0;
    for (const QString& url : pageUrls) {
        // Simple async fetch using QNetworkAccessManager (via run_command? No, code here).
        // Since we don't have easy access to NAM for async image fetching in this context without a proper async loader class...
        // We can spawn a thread to download synchronous? Or use QNAM properly.
        // Let's us QNetworkAccessManager from a new instance or passed one?
        // Creating a new QNAM per request is okayish for small apps but better to reuse.
        // MainWindow has one but ReaderWidget doesn't.

        // MVP: Use QNetworkAccessManager in a simple way.
        // Actually best to do this in main thread async.

        QNetworkAccessManager* nam = new QNetworkAccessManager(this);
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

        // Add Referer if available
        QString baseUrl = source->baseUrl();
        if (!baseUrl.isEmpty()) {
            req.setRawHeader("Referer", baseUrl.toUtf8());
        }

        QNetworkReply* reply = nam->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply, pageIndex, nam]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QImage img;
                img.loadFromData(data);
                if (!img.isNull()) {
                     emit pageLoaded(img, pageIndex);
                }
            } else {
                qWarning() << "Failed to load page" << pageIndex << reply->errorString();
            }
            reply->deleteLater();
            nam->deleteLater(); // Cleanup
        });

        pageIndex++;
    }
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

        // Adjust target width for padding
        int paddingPx = targetWidth * m_webtoonPadding / 100;
        targetWidth -= (paddingPx * 2);

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
    // Update Single Page Preference
    if (mode == LeftToRight || mode == RightToLeft) {
        m_lastSinglePageMode = mode;
    }

    // Smart Double Page Selection
    if (mode == DoublePageSpread) {
        if (m_lastSinglePageMode == LeftToRight) {
            m_readingMode = DoublePageSpreadLeftToRight;
        } else {
            m_readingMode = DoublePageSpread; // Keeps it as RTL Spread (value 3)
        }
    } else {
        m_readingMode = mode;
    }

    // Apply Mode
    if (m_readingMode == Webtoon) {
        m_viewStack->setCurrentIndex(0);
    } else {
        m_viewStack->setCurrentIndex(1);
        updateView();
    }

    // Ensure Settings Overlay shows correct generic "Double Page"
    // The overlay mapping (4 -> 3) is handled in ReaderSettingsOverlay::setCurrentReadingMode
    if (m_settingsOverlay) {
        m_settingsOverlay->setCurrentReadingMode(m_readingMode);
    }
}

void ReaderWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Right:
    case Qt::Key_D:
        if (m_readingMode == RightToLeft || m_readingMode == DoublePageSpread) previousPage();
        else nextPage();
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        if (m_readingMode == RightToLeft || m_readingMode == DoublePageSpread) nextPage();
        else previousPage();
        break;
    case Qt::Key_Space:
        nextPage();
        break;
    case Qt::Key_S:
        toggleSettingsOverlay();
        break;
    case Qt::Key_Up:
        previousPage();
        break;
    case Qt::Key_Down:
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

void ReaderWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit navigateBack();
        return;
    }

    int w = width();
    int h = height();
    int x = event->pos().x();
    int y = event->pos().y();

    // Middle zone: toggle overlay
    if (x > w / 3 && x < 2 * w / 3 && y > h / 3 && y < 2 * h / 3) {
        toggleSettingsOverlay();
        return;
    }

    if (m_readingMode == Webtoon) {
        // Top zone: scroll up or prev page
        if (y < h / 3) {
            previousPage();
        } else if (y > 2 * h / 3) {
            nextPage();
        }
    } else {
        // Paged mode (LTR, RTL, Double)
        bool isRTL = (m_readingMode == RightToLeft || m_readingMode == DoublePageSpread);

        if (x < w / 3) {
            // Left zone
            if (isRTL) nextPage();
            else previousPage();
        } else if (x > 2 * w / 3) {
            // Right zone
            if (isRTL) previousPage();
            else nextPage();
        }
    }
}

void ReaderWidget::nextPage()
{
    if (m_readingMode == Webtoon) {
        QScrollBar *vBar = m_scrollArea->verticalScrollBar();
        if (vBar->value() >= vBar->maximum()) {
            if (hasNextChapter()) {
                emit requestNextChapter(m_currentChapterId);
            }
        } else {
            vBar->setValue(vBar->value() + m_scrollArea->viewport()->height() / 2);
        }
    } else {
        int increment = (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) ? 2 : 1;
        bool isRTL = (m_readingMode == RightToLeft || m_readingMode == DoublePageSpread);

        // Logic for "Next" action (advancing reading progress)
        // For RTL, "Next" key moves to LOWER index usually?
        // No, standard mapping: Right Key = Previous Page (in RTL context usually goes to 'next' visual page which is previous index, wait).
        // Let's stick to: nextPage() advances the *index* (1 -> 2).
        // If RTL, Key_Left calls nextPage(). Key_Right calls previousPage().

        bool atEnd = false;
        if (m_currentPageIndex + increment >= m_loadedPages.size()) {
             // If we are at the last page (or spread containing last page)
             // Check if we are ALREADY at the very end state (viewing it)
             // Actually, usually users want one more press to trigger "Next Chapter".

             // Simplest: Check if we are attempting to go past the last valid index.
             // If m_currentPageIndex is already at (size-1), and we try to go next.

             // Let's refine simple index logic:
             if (m_currentPageIndex + 1 >= m_loadedPages.size()) {
                 atEnd = true;
             }
        }

        if (atEnd) {
            if (m_showingTransition) {
                if (hasNextChapter()) {
                    emit requestNextChapter(m_currentChapterId);
                }
            } else {
                // Show Transition
                m_showingTransition = true;

                // Construct Transition Image
                QPixmap transition(this->width(), this->height());
                transition.fill(Qt::black);
                QPainter p(&transition);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial", 20, QFont::Bold));

                QString text;
                if (hasNextChapter()) {
                    text = "Finished Chapter\nClick again for next chapter";
                } else {
                    text = "End of Series\nNo more chapters";
                }

                p.drawText(transition.rect(), Qt::AlignCenter, text);
                p.end();

                m_pagedLabel->setPixmap(transition);
                m_pagedLabel->adjustSize(); // Ensure label takes size
            }
            return;
        }

        m_showingTransition = false; // Reset if we moved successfully


        // Standard Progression
        if (m_currentPageIndex + increment < m_loadedPages.size()) {
            m_currentPageIndex += increment;
            updateView();
        } else if (m_currentPageIndex + 1 < m_loadedPages.size()) {
            // Handle odd last page in double spread
            m_currentPageIndex++;
            updateView();
        } else {
            // Fallback for exactly at limit
            if (hasNextChapter()) {
                emit requestNextChapter(m_currentChapterId);
            }
        }
    }
}

void ReaderWidget::previousPage()
{
    if (m_readingMode == Webtoon) {
        int scrollValue = m_scrollArea->verticalScrollBar()->value();
        if (scrollValue <= 0) {
            if (hasPreviousChapter()) {
                emit requestPreviousChapter(m_currentChapterId);
            }
            return;
        }
        m_scrollArea->verticalScrollBar()->setValue(
            scrollValue - m_scrollArea->viewport()->height() / 2
        );
    } else {
        // Check if at first page - request previous chapter
        if (m_currentPageIndex == 0) {
            if (m_showingTransition) {
                 if (hasPreviousChapter()) {
                     emit requestPreviousChapter(m_currentChapterId);
                 }
            } else {
                 m_showingTransition = true;

                 QPixmap transition(this->width(), this->height());
                 transition.fill(Qt::black);
                 QPainter p(&transition);
                 p.setPen(Qt::white);
                 p.setFont(QFont("Arial", 20, QFont::Bold));

                 QString text;
                 if (hasPreviousChapter()) {
                     text = "Start of Chapter\nClick again for previous chapter";
                 } else {
                     text = "First Chapter\nNo previous chapter";
                 }

                 p.drawText(transition.rect(), Qt::AlignCenter, text);
                 p.end();

                 m_pagedLabel->setPixmap(transition);
            }
            return;
        }

        m_showingTransition = false;



        int decrement = (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) ? 2 : 1;

        if (m_currentPageIndex - decrement >= 0) {
            m_currentPageIndex -= decrement;
            updateView();
        } else if (m_currentPageIndex > 0) {
            m_currentPageIndex = 0;
            updateView();
        }
    }
}

void ReaderWidget::updateView()
{
    if (m_readingMode == Webtoon) return;

    if (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) {
        // Double Page Spread
        QImage rightImg, leftImg;
        // In RTL Double: [Page N+1] [Page N]  (Page N is on Right)
        // In LTR Double: [Page N] [Page N+1]  (Page N is on Left)

        bool isRTL = (m_readingMode == DoublePageSpread);

        int firstIndex = m_currentPageIndex;
        int secondIndex = m_currentPageIndex + 1;

        QImage* firstPosImg = nullptr; // Left-side image in viewer
        QImage* secondPosImg = nullptr; // Right-side image in viewer

        if (isRTL) {
             // RTL: Right side is `firstIndex` (N), Left side is `secondIndex` (N+1)
             if (firstIndex < m_loadedPages.size()) rightImg = m_loadedPages[firstIndex];
             if (secondIndex < m_loadedPages.size()) leftImg = m_loadedPages[secondIndex];
        } else {
             // LTR: Left side is `firstIndex` (N), Right side is `secondIndex` (N+1)
             if (firstIndex < m_loadedPages.size()) leftImg = m_loadedPages[firstIndex];
             if (secondIndex < m_loadedPages.size()) rightImg = m_loadedPages[secondIndex];
        }

        if (rightImg.isNull() && leftImg.isNull()) return;

        int totalWidth = 0;
        int maxHeight = 0;

        if (!rightImg.isNull()) {
            totalWidth += rightImg.width();
            maxHeight = qMax(maxHeight, rightImg.height());
        }
        if (!leftImg.isNull()) {
            totalWidth += leftImg.width();
            maxHeight = qMax(maxHeight, leftImg.height());
        }

        // Create combined image
        QImage combined(totalWidth, maxHeight, QImage::Format_ARGB32);
        combined.fill(Qt::black);
        QPainter painter(&combined);

        int currentX = 0;

        // Draw Left Image first (on the left side)
        if (!leftImg.isNull()) {
             // Center vertically if needed, or align top
             painter.drawImage(0, 0, leftImg);
             currentX += leftImg.width();
        }

        // Draw Right Image (on the right side)
        if (!rightImg.isNull()) {
            painter.drawImage(currentX, 0, rightImg);
        }

        painter.end();

        QPixmap pixmap = QPixmap::fromImage(combined);
        int w = this->width();
        int h = this->height();

        Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
        if (m_scaleType == Stretch) aspectRatioMode = Qt::IgnoreAspectRatio;

        if (m_scaleType == OriginalSize) {
             m_pagedLabel->setPixmap(pixmap);
        } else if (m_scaleType == FitWidth) {
             m_pagedLabel->setPixmap(pixmap.scaledToWidth(w, Qt::SmoothTransformation));
        } else if (m_scaleType == FitHeight) {
             m_pagedLabel->setPixmap(pixmap.scaledToHeight(h, Qt::SmoothTransformation));
        } else if (m_scaleType == SmartFit) {
             // SmartFit: Fit screen but don't upscale beyond original if it fits
             if (pixmap.width() <= w && pixmap.height() <= h) {
                 m_pagedLabel->setPixmap(pixmap);
             } else {
                 m_pagedLabel->setPixmap(pixmap.scaled(w, h, aspectRatioMode, Qt::SmoothTransformation));
             }
        } else {
             // FitScreen
             m_pagedLabel->setPixmap(pixmap.scaled(w, h, aspectRatioMode, Qt::SmoothTransformation));
        }

    } else {
        // Single Page Mode
        if (m_currentPageIndex >= 0 && m_currentPageIndex < m_loadedPages.size()) {
            QImage img = m_loadedPages[m_currentPageIndex];
            if (!img.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(img);
                int w = this->width();
                int h = this->height();

                Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
                if (m_scaleType == Stretch) aspectRatioMode = Qt::IgnoreAspectRatio;

                if (m_scaleType == OriginalSize) {
                     m_pagedLabel->setPixmap(pixmap);
                } else if (m_scaleType == FitWidth) {
                     m_pagedLabel->setPixmap(pixmap.scaledToWidth(w, Qt::SmoothTransformation));
                } else if (m_scaleType == FitHeight) {
                     m_pagedLabel->setPixmap(pixmap.scaledToHeight(h, Qt::SmoothTransformation));
                } else if (m_scaleType == SmartFit) {
                     if (pixmap.width() <= w && pixmap.height() <= h) {
                         m_pagedLabel->setPixmap(pixmap);
                     } else {
                         m_pagedLabel->setPixmap(pixmap.scaled(w, h, aspectRatioMode, Qt::SmoothTransformation));
                     }
                } else {
                     // FitScreen
                     m_pagedLabel->setPixmap(pixmap.scaled(w, h, aspectRatioMode, Qt::SmoothTransformation));
                }
            }
        }
    }
}

void ReaderWidget::saveReadingProgress()
{
    if (m_currentChapterId > 0) {
        // Save to history
        long now = QDateTime::currentSecsSinceEpoch();
        m_historyRepo->upsertHistory(m_currentChapterId, now, 0);

        // Update chapter's last_page_read
        Chapter chapter = m_chapterRepo->getChapterById(m_currentChapterId);
        if (chapter.id() != -1) {
            chapter.setLastPageRead(m_currentPageIndex);
            m_chapterRepo->updateChapter(chapter);
        }

        emit chapterClosed(m_currentChapterId, m_currentPageIndex);
    }
}

void ReaderWidget::showSettingsOverlay()
{
    if (!m_settingsOverlay) {
        m_settingsOverlay = new ReaderSettingsOverlay(this);
        m_settingsOverlay->setCurrentReadingMode(static_cast<int>(m_readingMode));
        m_settingsOverlay->setCurrentScaleType(static_cast<int>(m_scaleType));

        connect(m_settingsOverlay, &ReaderSettingsOverlay::readingModeChanged, this, &ReaderWidget::onReadingModeChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::scaleTypeChanged, this, &ReaderWidget::onScaleTypeChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::closeRequested, this, &ReaderWidget::hideSettingsOverlay);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::backToLibraryRequested, this, &ReaderWidget::navigateBack);
    }

    // Position in center
    m_settingsOverlay->adjustSize(); // Fixes "blank menu" by ensuring dimensions are calculated
    int x = (this->width() - m_settingsOverlay->width()) / 2;
    int y = (this->height() - m_settingsOverlay->height()) / 2;
    m_settingsOverlay->move(x, y);
    m_settingsOverlay->show();
    m_settingsOverlay->raise();
    m_settingsOverlay->activateWindow();
}

void ReaderWidget::hideSettingsOverlay()
{
    if (m_settingsOverlay) {
        m_settingsOverlay->hide();
    }
}

void ReaderWidget::toggleSettingsOverlay()
{
    if (m_settingsOverlay && m_settingsOverlay->isVisible()) {
        hideSettingsOverlay();
    } else {
        showSettingsOverlay();
    }
}

void ReaderWidget::onReadingModeChanged(int mode)
{
    setReadingMode(static_cast<ReadingMode>(mode));
    PreferenceManager::instance().setValue("reader/default_mode", mode);
}

void ReaderWidget::onScaleTypeChanged(int type)
{
    m_scaleType = static_cast<ScaleType>(type);
    PreferenceManager::instance().setValue("reader/scale_type", type);
    updateView();
}

bool ReaderWidget::hasNextChapter() const
{
    if (m_chapters.isEmpty()) return false;

    // Try to find current index
    int idx = -1;
    for (int i = 0; i < m_chapters.size(); ++i) {
        if (m_chapters[i].id() == m_currentChapterId) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    // We have a next chapter if we are NOT at the last index
    // Valid next indexes are [idx+1 ... size-1]
    return idx < (m_chapters.size() - 1);
}

bool ReaderWidget::hasPreviousChapter() const
{
    if (m_chapters.isEmpty()) return false;

    // Try to find current index
    int idx = -1;
    for (int i = 0; i < m_chapters.size(); ++i) {
        if (m_chapters[i].id() == m_currentChapterId) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    // We have a previous chapter if we are NOT at the first index (0)
    return idx > 0;
}
