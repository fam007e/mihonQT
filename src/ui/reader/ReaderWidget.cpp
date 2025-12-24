#include "ReaderWidget.h"
#include "ReaderSettingsOverlay.h"
#include "../../model/Manga.h"
#include "../../model/Chapter.h"
#include "../../source/LocalSource.h"
#include "../../database/ChapterRepository.h"
#include "../../database/MangaRepository.h"
#include "../../database/HistoryRepository.h"
#include "../../database/DatabaseManager.h"
#include "../../config/PreferenceManager.h"

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
#include <QMouseEvent>

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
      m_chapterRepo(new ChapterRepository()),
      m_mangaRepo(new MangaRepository()),
      m_historyRepo(new HistoryRepository()),
      m_settingsOverlay(nullptr)
{
    reloadSettings();

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
    delete m_localSource;
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
        m_currentChapterId = chapterToLoad.id();
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
    m_readingMode = mode;
    if (m_readingMode == Webtoon) {
        m_viewStack->setCurrentIndex(0);
    } else {
        m_viewStack->setCurrentIndex(1);
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
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->value() + m_scrollArea->viewport()->height() / 2
        );
    } else {
        int increment = (m_readingMode == DoublePageSpread) ? 2 : 1;

        if (m_readingMode == RightToLeft || m_readingMode == DoublePageSpread) {
             // For RightToLeft, "Next" usually means "Previous Index" (reading backwards)
             // BUT, usually navigation keys are mapped: Right Arrow -> Next Page (visually next)
             // In standard manga readers:
             // Tap Left -> Next Page (Index + 1)
             // Tap Right -> Previous Page (Index - 1)
             // Let's stick to the keyPressEvent logic:
             // Key_Right -> if RTL, previousPage()
             // Key_Left -> if RTL, nextPage()

             // So nextPage() here means "Advance in content" (Index + 1 or + 2)
             if (m_currentPageIndex + increment < m_loadedPages.size()) {
                m_currentPageIndex += increment;
                updateView();
            } else if (m_currentPageIndex + 1 < m_loadedPages.size()) {
                 // Handle odd last page
                 m_currentPageIndex++;
                 updateView();
            }
        } else {
            // LTR
            if (m_currentPageIndex + increment < m_loadedPages.size()) {
                m_currentPageIndex += increment;
                updateView();
            }
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
        int decrement = (m_readingMode == DoublePageSpread) ? 2 : 1;

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

    if (m_readingMode == DoublePageSpread) {
        // Double Page Spread (Right-to-Left: [Page N+1] [Page N])
        QImage rightImg, leftImg;

        // Current index is the "Right" page (Page N)
        if (m_currentPageIndex < m_loadedPages.size()) {
            rightImg = m_loadedPages[m_currentPageIndex];
        }

        // Next index is the "Left" page (Page N+1)
        if (m_currentPageIndex + 1 < m_loadedPages.size()) {
            leftImg = m_loadedPages[m_currentPageIndex + 1];
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
