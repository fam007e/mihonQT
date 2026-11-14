#include "ReaderWidget.h"
#include "ReaderSettingsOverlay.h"
#include "ReaderBarsOverlay.h"
#include "../ThemeManager.h"
#include "../../model/Manga.h"
#include "../../model/Chapter.h"
#include "../../model/ChapterSort.h"
#include "../../util/ImageUtil.h"
#include "../../network/NetworkAccessManager.h"
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
      m_localSource(new LocalSource()),
      m_sourceManager(sourceManager),
      m_chapterRepo(new ChapterRepository()),
      m_mangaRepo(new MangaRepository()),
      m_historyRepo(new HistoryRepository()),
      m_settingsOverlay(nullptr),
      m_showingTransition(false),
      m_firstPageCover(true),
      m_autoWideDetect(true)
{
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
    m_viewStack->setCurrentIndex(m_readingMode == Webtoon ? 0 : 1);

    connect(this, &ReaderWidget::pageLoaded, this, &ReaderWidget::displayPage);

    setFocusPolicy(Qt::StrongFocus); // Enable focus for key events

    reloadSettings();

    // --- Mihon-style reader chrome ---
    // Dim layer for custom brightness (kept above pages, below overlays)
    m_brightnessOverlay = new QWidget(this);
    m_brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_brightnessOverlay->setGeometry(rect());

    // Top/bottom bars overlay (back, chapter title, page slider, chapter skip)
    m_barsOverlay = new ReaderBarsOverlay(this);
    m_barsOverlay->setGeometry(rect());
    connect(m_barsOverlay, &ReaderBarsOverlay::backRequested, this, &ReaderWidget::navigateBack);
    connect(m_barsOverlay, &ReaderBarsOverlay::previousChapterRequested, this, [this]() {
        if (hasPreviousChapter()) emit requestPreviousChapter(m_currentChapterId);
    });
    connect(m_barsOverlay, &ReaderBarsOverlay::nextChapterRequested, this, [this]() {
        if (hasNextChapter()) emit requestNextChapter(m_currentChapterId);
    });
    connect(m_barsOverlay, &ReaderBarsOverlay::pageSelected, this, [this](int page) {
        page = qBound(0, page, m_loadedPages.size() - 1);
        m_currentPageIndex = page;
        if (m_readingMode == Webtoon) {
            if (page >= 0 && page < m_webtoonLabels.size() && m_webtoonLabels[page])
                m_scrollArea->ensureWidgetVisible(m_webtoonLabels[page]);
        } else {
            updateView();
        }
        updateBarsOverlay();
    });

    m_brightness = PreferenceManager::instance().value("reader/brightness", 100).toInt();
    applyBrightness();
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
    m_fullscreen = PreferenceManager::instance().value("reader/fullscreen", false).toBool();
    m_firstPageCover = PreferenceManager::instance().value("reader/double_page_cover", true).toBool();
    m_autoWideDetect = PreferenceManager::instance().value("reader/double_page_auto_wide", true).toBool();

    int savedMode = PreferenceManager::instance().value("reader/default_mode", 1).toInt(); // Default to LeftToRight (1)
    setReadingMode(static_cast<ReadingMode>(savedMode));

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
    // Ensure chapters are sorted in ascending reading order (matching Android Mihon ReaderViewModel)
    auto compInit = getChapterComparator(m_currentManga, /*sortDescending=*/false);
    std::sort(m_chapters.begin(), m_chapters.end(), compInit);

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
    // Ensure chapters are sorted in ascending reading order (matching Android Mihon ReaderViewModel)
    auto comp = getChapterComparator(m_currentManga, /*sortDescending=*/false);
    std::sort(m_chapters.begin(), m_chapters.end(), comp);

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
    m_webtoonLabels.clear();
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

    if (m_currentManga.source() != 0 || chapterUrl.startsWith("http://") || chapterUrl.startsWith("https://") || chapterUrl.startsWith("/")) {
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

    // Resize loaded pages vector and setup webtoon placeholders
    m_loadedPages.resize(pageUrls.size());
    for (int i = 0; i < pageUrls.size(); ++i) {
        QLabel* label = new QLabel(m_contentWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("background-color: black;");
        label->setScaledContents(false);
        m_contentLayout->addWidget(label);
        m_webtoonLabels.append(label);
    }

    if (startAtEnd && !m_loadedPages.isEmpty()) {
        int target = m_loadedPages.size() - 1;
        if (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) {
            int spreadAnchor = 0;
            while (spreadAnchor < target) {
                SpreadInfo sInfo = getSpreadInfo(spreadAnchor);
                if (spreadAnchor + sInfo.stepForward > target) {
                    break;
                }
                spreadAnchor += sInfo.stepForward;
            }
            m_currentPageIndex = spreadAnchor;
        } else {
            m_currentPageIndex = target;
        }
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
    QList<QString> pageUrls = source->getPageList(chapter);

    if (pageUrls.isEmpty()) {
         qDebug() << "ReaderWidget: No pages found from source.";
         QLabel* label = new QLabel("No pages returned from source.", m_contentWidget);
         m_contentLayout->addWidget(label);
         return;
    }

    qDebug() << "ReaderWidget: Loaded" << pageUrls.size() << "page URLs from source.";
    m_loadedPages.resize(pageUrls.size());
    for (int i = 0; i < pageUrls.size(); ++i) {
        QLabel* label = new QLabel(m_contentWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("background-color: black;");
        label->setScaledContents(false);
        m_contentLayout->addWidget(label);
        m_webtoonLabels.append(label);
    }

    // Mihon resume behaviour (when not explicitly jumping to an end):
    // a partially read chapter opens at its saved page; a chapter read to
    // the end continues straight into the next one.
    if (!startAtEnd && pageUrls.size() > 0) {
        const int saved = chapter.lastPageRead();
        if (saved >= pageUrls.size() - 1) {
            for (int i = 0; i < m_chapters.size(); ++i) {
                if (m_chapters[i].id() == chapter.id()) {
                    if (i + 1 < m_chapters.size()) {
                        Chapter next = m_chapters[i + 1];
                        qDebug() << "ReaderWidget: chapter already finished, continuing with:" << next.name();
                        m_currentChapterId = next.id();
                        loadAndDisplayPages(next, false);
                        return;
                    }
                    break; // last chapter — stay on its final page
                }
            }
        } else if (saved > 0) {
            m_currentPageIndex = qMin(saved, pageUrls.size() - 1);
            qDebug() << "ReaderWidget: resuming at page" << m_currentPageIndex + 1;
        }
    }

    if (startAtEnd && !m_loadedPages.isEmpty()) {
        int target = m_loadedPages.size() - 1;
        if (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) {
            int spreadAnchor = 0;
            while (spreadAnchor < target) {
                SpreadInfo sInfo = getSpreadInfo(spreadAnchor);
                if (spreadAnchor + sInfo.stepForward > target) {
                    break;
                }
                spreadAnchor += sInfo.stepForward;
            }
            m_currentPageIndex = spreadAnchor;
        } else {
            m_currentPageIndex = target;
        }
        updateView();
    }

    // Shared QNAM for reading with cookies and connection reuse
    static QNetworkAccessManager* s_readerNam = nullptr;
    if (!s_readerNam) {
        s_readerNam = new QNetworkAccessManager(qApp);
        s_readerNam->setCookieJar(NetworkAccessManager::sharedCookieJar());
        NetworkAccessManager::sharedCookieJar()->setParent(qApp);
    }

    int pageIndex = 0;
    for (const QString& url : pageUrls) {
        QString fullPageUrl = url;
        QString baseUrl = source->baseUrl();
        if (fullPageUrl.startsWith("/")) {
            fullPageUrl = baseUrl + fullPageUrl;
        }

        QNetworkRequest req{QUrl(fullPageUrl)};
        req.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

        if (!baseUrl.isEmpty()) {
            req.setRawHeader("Referer", baseUrl.toUtf8());
        }

        QNetworkReply* reply = s_readerNam->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply, pageIndex]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QImage img = ImageUtil::loadFromData(data); // WebP support!
                if (!img.isNull()) {
                     emit pageLoaded(img, pageIndex);
                } else {
                     qWarning() << "ReaderWidget: Failed to decode page" << pageIndex << "image data size:" << data.size();
                }
            } else {
                qWarning() << "ReaderWidget: Failed to download page" << pageIndex << reply->errorString();
            }
            reply->deleteLater();
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
        if (pageIndex >= 0 && pageIndex < m_webtoonLabels.size() && m_webtoonLabels[pageIndex]) {
            QPixmap pixmap = QPixmap::fromImage(image);
            int targetWidth = m_scrollArea->viewport()->width();
            if (targetWidth <= 0) targetWidth = this->width();

            int paddingPx = targetWidth * m_webtoonPadding / 100;
            targetWidth -= (paddingPx * 2);

            if (targetWidth > 0 && pixmap.width() > targetWidth) {
                pixmap = pixmap.scaledToWidth(targetWidth, Qt::SmoothTransformation);
            }

            m_webtoonLabels[pageIndex]->setPixmap(pixmap);
        }
    } else {
        // For Paged mode (LTR, RTL, Double Page Spread):
        SpreadInfo info = getSpreadInfo(m_currentPageIndex);
        if (pageIndex == m_currentPageIndex || (info.secondIndex != -1 && pageIndex == info.secondIndex)) {
            updateView();
        }
    }

    updateBarsOverlay();
}

void ReaderWidget::updateWebtoonView()
{
    int targetWidth = m_scrollArea->viewport()->width();
    if (targetWidth <= 0) targetWidth = this->width();
    int paddingPx = targetWidth * m_webtoonPadding / 100;
    targetWidth -= (paddingPx * 2);

    for (int i = 0; i < m_loadedPages.size(); ++i) {
        if (i < m_webtoonLabels.size() && m_webtoonLabels[i]) {
            const QImage& img = m_loadedPages[i];
            if (!img.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(img);
                if (targetWidth > 0 && pixmap.width() > targetWidth) {
                    pixmap = pixmap.scaledToWidth(targetWidth, Qt::SmoothTransformation);
                }
                m_webtoonLabels[i]->setPixmap(pixmap);
            }
        }
    }
}

void ReaderWidget::setReadingMode(ReadingMode mode)
{
    m_readingMode = mode;

    // Apply Mode
    if (m_viewStack) {
        if (m_readingMode == Webtoon) {
            m_viewStack->setCurrentIndex(0);
            updateWebtoonView();
        } else {
            m_viewStack->setCurrentIndex(1);
            updateView();
        }
    }

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
        // Mihon: Esc closes the open overlay first, then exits the reader
        if ((m_settingsOverlay && m_settingsOverlay->isVisible()) ||
            (m_barsOverlay && m_barsOverlay->isVisible())) {
            hideSettingsOverlay();
        } else {
            emit navigateBack();
        }
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

ReaderWidget::SpreadInfo ReaderWidget::getSpreadInfo(int index) const
{
    SpreadInfo info;
    if (index < 0 || index >= m_loadedPages.size()) return info;

    info.firstIndex = index;
    info.secondIndex = -1;
    info.stepForward = 1;
    info.isSingle = true;

    if (m_readingMode != DoublePageSpread && m_readingMode != DoublePageSpreadLeftToRight) {
        return info;
    }

    // Cover page offset: Page 0 is kept single if enabled
    if (index == 0 && m_firstPageCover) {
        info.isSingle = true;
        info.stepForward = 1;
        return info;
    }

    const QImage& currentImg = m_loadedPages[index];
    // If current page is wide landscape, display alone
    if (m_autoWideDetect && !currentImg.isNull() && currentImg.width() > currentImg.height()) {
        info.isSingle = true;
        info.stepForward = 1;
        return info;
    }

    // Try to pair with the next page
    int nextIndex = index + 1;
    if (nextIndex < m_loadedPages.size()) {
        const QImage& nextImg = m_loadedPages[nextIndex];
        // Don't pair if partner is a wide landscape image
        if (m_autoWideDetect && !nextImg.isNull() && nextImg.width() > nextImg.height()) {
            info.isSingle = true;
            info.stepForward = 1;
            return info;
        }

        info.secondIndex = nextIndex;
        info.stepForward = 2;
        info.isSingle = false;
    }

    return info;
}

int ReaderWidget::getPreviousSpreadIndex(int currentIndex) const
{
    if (currentIndex <= 0) return 0;

    // Check if index - 2 was a 2-page spread that stepped forward to currentIndex
    if (currentIndex - 2 >= 0) {
        SpreadInfo pInfo = getSpreadInfo(currentIndex - 2);
        if (currentIndex - 2 + pInfo.stepForward == currentIndex) {
            return currentIndex - 2;
        }
    }

    // Fallback: previous spread is 1 page behind
    if (currentIndex - 1 >= 0) {
        return currentIndex - 1;
    }

    return 0;
}

void ReaderWidget::onFirstPageCoverChanged(bool enabled)
{
    m_firstPageCover = enabled;
    PreferenceManager::instance().setValue("reader/double_page_cover", enabled);
    updateView();
}

void ReaderWidget::onAutoWideDetectChanged(bool enabled)
{
    m_autoWideDetect = enabled;
    PreferenceManager::instance().setValue("reader/double_page_auto_wide", enabled);
    updateView();
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
        int increment = 1;
        if (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) {
            SpreadInfo info = getSpreadInfo(m_currentPageIndex);
            increment = info.stepForward;
        }

        bool atEnd = (m_currentPageIndex + increment >= m_loadedPages.size());

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

        m_currentPageIndex += increment;
        updateView();
        saveReadingProgress();
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

        if (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) {
            m_currentPageIndex = getPreviousSpreadIndex(m_currentPageIndex);
        } else {
            m_currentPageIndex = qMax(0, m_currentPageIndex - 1);
        }

        updateView();
        saveReadingProgress();
    }
}

void ReaderWidget::updateView()
{
    if (m_readingMode == Webtoon) return;

    QPixmap pixmap;

    if (m_readingMode == DoublePageSpread || m_readingMode == DoublePageSpreadLeftToRight) {
        SpreadInfo info = getSpreadInfo(m_currentPageIndex);
        if (info.firstIndex < 0 || info.firstIndex >= m_loadedPages.size()) return;

        bool isRTL = (m_readingMode == DoublePageSpread);
        QImage mainImg = m_loadedPages[info.firstIndex];
        if (mainImg.isNull()) return;

        if (info.isSingle || info.secondIndex < 0 || info.secondIndex >= m_loadedPages.size() || m_loadedPages[info.secondIndex].isNull()) {
            pixmap = QPixmap::fromImage(mainImg);
        } else {
            QImage partnerImg = m_loadedPages[info.secondIndex];
            QImage leftImg = isRTL ? partnerImg : mainImg;
            QImage rightImg = isRTL ? mainImg : partnerImg;

            int totalWidth = leftImg.width() + rightImg.width();
            int maxHeight = qMax(leftImg.height(), rightImg.height());

            QImage combined(totalWidth, maxHeight, QImage::Format_ARGB32);
            combined.fill(Qt::black);
            QPainter painter(&combined);

            int yLeft = (maxHeight - leftImg.height()) / 2;
            int yRight = (maxHeight - rightImg.height()) / 2;

            painter.drawImage(0, yLeft, leftImg);
            painter.drawImage(leftImg.width(), yRight, rightImg);
            painter.end();

            pixmap = QPixmap::fromImage(combined);
        }
    } else {
        if (m_currentPageIndex >= 0 && m_currentPageIndex < m_loadedPages.size()) {
            QImage img = m_loadedPages[m_currentPageIndex];
            if (!img.isNull()) {
                pixmap = QPixmap::fromImage(img);
            }
        }
    }

    if (pixmap.isNull()) {
        if (!m_loadedPages.isEmpty()) {
            m_pagedLabel->setText(QString("Loading page %1 / %2...").arg(m_currentPageIndex + 1).arg(m_loadedPages.size()));
            m_pagedLabel->setStyleSheet("color: #888888; font-size: 16px; background-color: black;");
        }
        return;
    }

    m_pagedLabel->setStyleSheet("background-color: black;");

    int w = m_pagedScrollArea->viewport()->width();
    int h = m_pagedScrollArea->viewport()->height();
    if (w <= 0) w = this->width();
    if (h <= 0) h = this->height();

    if (w <= 50 || h <= 50) {
        m_pagedLabel->setPixmap(pixmap);
        return;
    }

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
        m_pagedLabel->setPixmap(pixmap.scaled(w, h, aspectRatioMode, Qt::SmoothTransformation));
    }
}

void ReaderWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_brightnessOverlay)
        m_brightnessOverlay->setGeometry(rect());
    if (m_barsOverlay)
        m_barsOverlay->setGeometry(rect());
    if (m_readingMode != Webtoon) {
        updateView();
    } else {
        updateWebtoonView();
    }
}

void ReaderWidget::saveReadingProgress()
{
    if (m_currentChapterId > 0) {
        // Save to history if NOT in incognito mode
        bool incognito = PreferenceManager::instance().value("security/incognito", false).toBool();
        if (!incognito) {
            long now = QDateTime::currentSecsSinceEpoch();
            m_historyRepo->upsertHistory(m_currentChapterId, now, 0);
        }

        // Update chapter's last_page_read (even in incognito? Mihon Android updates local progress but not global history)
        // Usually incognito means "don't save anything that reveals what I read".
        if (!incognito) {
            Chapter chapter = m_chapterRepo->getChapterById(m_currentChapterId);
            if (chapter.id() != -1) {
                chapter.setLastPageRead(m_currentPageIndex);
                m_chapterRepo->updateChapter(chapter);
            }
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
        m_settingsOverlay->setFirstPageCover(m_firstPageCover);
        m_settingsOverlay->setAutoWideDetect(m_autoWideDetect);

        connect(m_settingsOverlay, &ReaderSettingsOverlay::readingModeChanged, this, &ReaderWidget::onReadingModeChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::scaleTypeChanged, this, &ReaderWidget::onScaleTypeChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::firstPageCoverChanged, this, &ReaderWidget::onFirstPageCoverChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::autoWideDetectChanged, this, &ReaderWidget::onAutoWideDetectChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::brightnessChanged, this, &ReaderWidget::onBrightnessChanged);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::closeRequested, this, &ReaderWidget::hideSettingsOverlay);
        connect(m_settingsOverlay, &ReaderSettingsOverlay::backToLibraryRequested, this, &ReaderWidget::navigateBack);
    } else {
        m_settingsOverlay->setCurrentReadingMode(static_cast<int>(m_readingMode));
        m_settingsOverlay->setCurrentScaleType(static_cast<int>(m_scaleType));
        m_settingsOverlay->setFirstPageCover(m_firstPageCover);
        m_settingsOverlay->setAutoWideDetect(m_autoWideDetect);
    }

    m_settingsOverlay->setCurrentBrightness(m_brightness);

    // Position in center
    m_settingsOverlay->adjustSize(); // Fixes "blank menu" by ensuring dimensions are calculated
    int x = (this->width() - m_settingsOverlay->width()) / 2;
    int y = (this->height() - m_settingsOverlay->height()) / 2;
    m_settingsOverlay->move(x, y);
    m_settingsOverlay->show();
    m_settingsOverlay->raise();
    m_settingsOverlay->activateWindow();

    // Show the Mihon-style bars chrome alongside the settings panel
    if (m_barsOverlay) {
        updateBarsOverlay();
        m_barsOverlay->setGeometry(rect());
        m_barsOverlay->show();
        m_barsOverlay->raise();
    }
}

void ReaderWidget::hideSettingsOverlay()
{
    if (m_settingsOverlay) {
        m_settingsOverlay->hide();
    }
    if (m_barsOverlay) {
        m_barsOverlay->hide();
    }
}

void ReaderWidget::onBrightnessChanged(int value)
{
    m_brightness = value;
    PreferenceManager::instance().setValue("reader/brightness", value);
    applyBrightness();
}

void ReaderWidget::applyBrightness()
{
    if (!m_brightnessOverlay) return;
    // 100 = no dimming; 10 = almost black (matches Mihon's custom brightness)
    int alpha = qBound(0, (100 - m_brightness) * 255 / 100, 255);
    if (alpha > 0) {
        QColor dim(Qt::black);
        dim.setAlpha(alpha);
        QPalette p = m_brightnessOverlay->palette();
        p.setColor(QPalette::Window, dim);
        m_brightnessOverlay->setAutoFillBackground(true);
        m_brightnessOverlay->setPalette(p);
        m_brightnessOverlay->show();
    } else {
        m_brightnessOverlay->hide();
    }
}

void ReaderWidget::updateBarsOverlay()
{
    if (!m_barsOverlay) return;

    // Chapter title
    for (const Chapter& ch : m_chapters) {
        if (ch.id() == m_currentChapterId) {
            m_barsOverlay->setChapterTitle(ch.name());
            break;
        }
    }

    m_barsOverlay->setPageInfo(m_currentPageIndex, m_loadedPages.size());
    bool rtl = (m_readingMode == RightToLeft || m_readingMode == DoublePageSpread);
    m_barsOverlay->setRtl(rtl);
    m_barsOverlay->setHasPreviousChapter(hasPreviousChapter());
    m_barsOverlay->setHasNextChapter(hasNextChapter());
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
