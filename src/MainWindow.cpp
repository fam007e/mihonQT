#include "MainWindow.h"
#include "database/DatabaseManager.h"
#include "database/MangaRepository.h"
#include "database/ChapterRepository.h"
#include "model/Manga.h"
#include "model/Chapter.h"
#include "source/LocalSource.h"
#include "ui/SourceListView.h"
#include "ui/MangaListView.h"
#include "ui/LibraryView.h"
#include "ui/UpdatesView.h"
#include "ui/HistoryView.h"
#include "ui/MangaDetailsView.h"
#include "ui/SettingsView.h"
#include "ui/ThemeManager.h"
#include "ui/SidebarWidget.h"

#include <QMenuBar>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QSettings>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_sourceManager(new SourceManager(this))
    , m_rootStack(new QStackedWidget(this)) // Root stack: Dashboard, Reader
    , m_dashboardWidget(new QWidget(this))  // Holds Sidebar + Content Stack
    , m_contentStack(new QStackedWidget(this)) // Content stack: Library, Updates, History, Browse, Details, Settings
    , m_jsEngine(new QJSEngine(this))
    , m_networkManager(new NetworkAccessManager(m_jsEngine, this))
{
    // Database setup
    if (QSqlDatabase::database().isOpen()) {
        qDebug() << "MainWindow: Database connection established.";
    } else {
        qDebug() << "MainWindow: Failed to establish database connection.";
        QMessageBox::critical(this, "Database Error", "Failed to establish database connection.");
    }

    // --- Initialize Sources ---
    QSettings settings("MihonQT", "MihonQT");
    QString localMangaPath = settings.value("localMangaPath", "").toString();
    LocalSource *localSource = new LocalSource(localMangaPath, this);
    m_sourceManager->addSource(localSource);

    JavascriptSource *jsSource = new JavascriptSource(
        "/home/fam007e/Github/mihonQT/src/sources/js/ExampleSource.js",
        m_jsEngine,
        m_networkManager,
        this
    );
    m_sourceManager->addSource(jsSource);

    // --- Initialize Components ---

    // 1. Library View
    m_libraryView = new LibraryView(this);
    connect(m_libraryView, &LibraryView::mangaSelected, this, &MainWindow::onMangaSelected);

    // 2. Browse View (Source List + Source Browse View)
    m_browseStack = new QStackedWidget(this);
    m_sourceListView = new SourceListView(m_sourceManager, this);
    m_sourceBrowseView = new SourceBrowseView(this);

    m_browseStack->addWidget(m_sourceListView);
    m_browseStack->addWidget(m_sourceBrowseView);

    connect(m_sourceListView, &SourceListView::sourceSelected, this, &MainWindow::onSourceSelected);
    connect(m_sourceBrowseView, &SourceBrowseView::mangaSelected, this, &MainWindow::onMangaSelected);
    connect(m_sourceBrowseView, &SourceBrowseView::backRequested, this, &MainWindow::showSourceList);

    // 3. New Views
    m_updatesView = new UpdatesView(this);
    m_historyView = new HistoryView(this);

    connect(m_libraryView, &LibraryView::mangaSelected, this, &MainWindow::onMangaSelected);
    connect(m_libraryView, &LibraryView::backRequested, this, &MainWindow::onBackRequested);
    connect(m_updatesView, &UpdatesView::chapterSelected, this, &MainWindow::onChapterSelected);


    m_mangaDetailsView = new MangaDetailsView(this);
    connect(m_mangaDetailsView, &MangaDetailsView::chapterSelected, this, &MainWindow::onChapterSelected);
    connect(m_mangaDetailsView, &MangaDetailsView::backRequested, this, &MainWindow::onBackRequested);
    connect(m_mangaDetailsView, &MangaDetailsView::libraryStatusChanged, m_libraryView, &LibraryView::refreshLibrary);

    // 5. Reader View
    m_readerWidget = new ReaderWidget(this);
    connect(m_readerWidget, &ReaderWidget::navigateBack, this, &MainWindow::onBackRequested);

    // 6. Settings View
    m_settingsView = new SettingsView(this);
    connect(m_settingsView, &SettingsView::backRequested, this, &MainWindow::onBackRequested);
connect(m_settingsView, &SettingsView::localMangaPathChanged, this, &MainWindow::onLocalMangaPathChanged);
    connect(m_settingsView, &SettingsView::readingModeChanged, this, [this](int index) {
        // 0: Webtoon, 1: L2R, 2: R2L
        ReaderWidget::ReadingMode mode;
        switch (index) {
            case 0: mode = ReaderWidget::Webtoon; break;
            case 1: mode = ReaderWidget::LeftToRight; break;
            case 2: mode = ReaderWidget::RightToLeft; break;
            default: mode = ReaderWidget::Webtoon; break;
        }
        m_readerWidget->setReadingMode(mode);
    });

    // --- Setup Sidebar Navigation ---
    m_sidebar = new SidebarWidget(this);
    connect(m_sidebar, &SidebarWidget::navigationRequested, this, &MainWindow::onNavigationRequested);

    // --- Setup Layout Structure ---

    // Content Stack (Right side)
    // Indexes:
    // 0: Library
    // 1: Updates
    // 2: History
    // 3: Browse
    // 4: Settings
    // 5: Details (Manga Details)

    m_contentStack->addWidget(m_libraryView);       // 0
    m_contentStack->addWidget(m_updatesView);       // 1
    m_contentStack->addWidget(m_historyView);       // 2
    m_contentStack->addWidget(m_browseStack);       // 3 (Browse acts as container for Source List/Manga List)
    m_contentStack->addWidget(m_settingsView);      // 4
    m_contentStack->addWidget(m_mangaDetailsView);  // 5

    // Dashboard Layout (Sidebar + Content)
    QHBoxLayout *dashboardLayout = new QHBoxLayout(m_dashboardWidget);
    dashboardLayout->setContentsMargins(0, 0, 0, 0);
    dashboardLayout->setSpacing(0);
    dashboardLayout->addWidget(m_sidebar);

    // Add a separator line if desired (optional)
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    // dashboardLayout->addWidget(line);

    dashboardLayout->addWidget(m_contentStack, 1); // Content takes available space

    // Root Stack (Dashboard + Reader)
    m_rootStack->addWidget(m_dashboardWidget); // Index 0
    m_rootStack->addWidget(m_readerWidget);    // Index 1

    setCentralWidget(m_rootStack);

    setupUi();

    if (menuBar()) {
        menuBar()->hide();
    }

    // Initial refresh
    m_libraryView->refreshLibrary();
    // Sources already populated via constructor now
}

MainWindow::~MainWindow()
{
    // m_sourceManager is a child of this, so it will be deleted automatically.
    // m_stackedWidget, m_sourceListView, m_mangaListView, m_readerWidget are also children of this
    // or m_stackedWidget, so they will be deleted automatically.
    // Database connection is managed by DatabaseManager singleton.
}

void MainWindow::setupUi()
{
    // Set the window title
    setWindowTitle("MihonQT");
    resize(1280, 720);

    // --- Toolbar & Hamburger Menu ---
    m_toolBar = addToolBar("Main Toolbar");
    m_toolBar->setMovable(false);

    // Hamburger Action (Icon would be better, using text for now)
    m_hamburgerAction = new QAction("Menu", this);
    // m_hamburgerAction->setIcon(QIcon(":/icons/menu.png")); // TODO: Add icon
    m_toolBar->addAction(m_hamburgerAction);

    // Spacer to push other items to right if needed
    // QWidget* spacer = new QWidget();
    // spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // m_toolBar->addWidget(spacer);

    // Only show toolbar if needed, or customize per view.
    // Ideally, for desktop, we might not need a top toolbar if we have a sidebar.
    // Or we keep it for global actions.
    m_toolBar->hide(); // Hiding for now to clean up UI, can re-enable if we want Breadcrumbs or Search

    // Hamburger Menu
    m_hamburgerMenu = new QMenu(this);

    // 1. Toggles
    m_downloadedOnlyAction = m_hamburgerMenu->addAction("Downloaded only");
    m_downloadedOnlyAction->setCheckable(true);

    m_incognitoModeAction = m_hamburgerMenu->addAction("Incognito mode");
    m_incognitoModeAction->setCheckable(true);

    m_hamburgerMenu->addSeparator();

    // 2. Shortcuts
    m_downloadQueueAction = m_hamburgerMenu->addAction("Download queue");
    m_categoriesAction = m_hamburgerMenu->addAction("Categories");
    m_statsAction = m_hamburgerMenu->addAction("Statistics");

    m_hamburgerMenu->addSeparator();

    // 3. Settings & About
    m_settingsAction = m_hamburgerMenu->addAction("Settings");
    m_aboutAction = m_hamburgerMenu->addAction("About");

    // Connect Hamburger Action to show menu
    connect(m_hamburgerAction, &QAction::triggered, this, [this]() {
        m_hamburgerMenu->exec(QCursor::pos());
    });

    // Connect Menu Actions (Placeholders for now)
    connect(m_downloadedOnlyAction, &QAction::toggled, this, [](bool checked) {
        qDebug() << "Downloaded Only:" << checked;
    });
    connect(m_incognitoModeAction, &QAction::toggled, this, [](bool checked) {
        qDebug() << "Incognito Mode:" << checked;
    });
    connect(m_downloadQueueAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Info", "Download Queue not implemented yet.");
    });
    connect(m_categoriesAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Info", "Categories not implemented yet.");
    });
    connect(m_statsAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "Info", "Statistics not implemented yet.");
    });
    connect(m_settingsAction, &QAction::triggered, this, [this]() {
        onNavigationRequested(4); // Navigate to Settings
    });
    connect(m_aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About MihonQT", "MihonQT v0.1\nA Qt port of the Mihon manga reader.");
    });

    // Create menu bar (Keep existing menus)
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // View Menu (Reading Modes)
    QMenu *viewMenu = menuBar()->addMenu("&View");
    QAction *webtoonAction = viewMenu->addAction("Webtoon Mode");
    QAction *l2rAction = viewMenu->addAction("Left to Right");
    QAction *r2lAction = viewMenu->addAction("Right to Left");

    connect(webtoonAction, &QAction::triggered, this, [this]() {
        m_readerWidget->setReadingMode(ReaderWidget::Webtoon);
    });
    connect(l2rAction, &QAction::triggered, this, [this]() {
        m_readerWidget->setReadingMode(ReaderWidget::LeftToRight);
    });
    connect(r2lAction, &QAction::triggered, this, [this]() {
        m_readerWidget->setReadingMode(ReaderWidget::RightToLeft);
    });

    // Theme Menu
    QMenu *themeMenu = menuBar()->addMenu("&Theme");
    QAction *nordDarkAction = themeMenu->addAction("Nord Dark");
    QAction *nordLightAction = themeMenu->addAction("Nord Light");
    QAction *catppuccinAction = themeMenu->addAction("Catppuccin Mocha");
    QAction *tokyoNightAction = themeMenu->addAction("Tokyo Night");
    QAction *draculaAction = themeMenu->addAction("Dracula");

    connect(nordDarkAction, &QAction::triggered, this, []() {
        ThemeManager::instance().applyTheme(ThemeManager::NordDark);
    });
    connect(nordLightAction, &QAction::triggered, this, []() {
        ThemeManager::instance().applyTheme(ThemeManager::NordLight);
    });
    connect(catppuccinAction, &QAction::triggered, this, []() {
        ThemeManager::instance().applyTheme(ThemeManager::CatppuccinMocha);
    });
    connect(tokyoNightAction, &QAction::triggered, this, []() {
        ThemeManager::instance().applyTheme(ThemeManager::TokyoNight);
    });
    connect(draculaAction, &QAction::triggered, this, []() {
        ThemeManager::instance().applyTheme(ThemeManager::Dracula);
    });

    // Apply default theme
    ThemeManager::instance().applyTheme(ThemeManager::NordDark);
}

void MainWindow::onSourceSelected(long sourceId)
{
    qDebug() << "Source selected:" << sourceId;
    showMangaList(sourceId);
}

void MainWindow::onMangaSelected(const Manga& manga)
{
    qDebug() << "Manga selected:" << manga.title();

    MangaRepository& mangaRepo = DatabaseManager::instance().mangaRepository();
    ChapterRepository& chapterRepo = DatabaseManager::instance().chapterRepository();
    SourceBase* source = m_sourceManager->getSourceById(manga.source());

    if (!source) {
        qWarning() << "Could not find source for manga" << manga.title();
        return;
    }

    // Check if manga is in DB by URL and source ID
    Manga dbManga = mangaRepo.getMangaByUrl(manga.url(), manga.source());

    long mangaIdToOpen = dbManga.id();

    if (mangaIdToOpen == -1) { // Manga not in DB, so add it
        qDebug() << "Manga not in library. Adding it...";
        // Fetch details before inserting
        Manga newManga = source->getMangaDetails(manga);
        newManga.setDateAdded(QDateTime::currentSecsSinceEpoch());

        if (mangaRepo.insertManga(newManga)) {
            // Re-get the manga to get its actual ID from the DB
            dbManga = mangaRepo.getMangaByUrl(newManga.url(), newManga.source());
            mangaIdToOpen = dbManga.id();
            qDebug() << "Inserted new manga with ID:" << mangaIdToOpen;

            // Now fetch and insert chapters for this new manga
            QList<SChapter> sChapters = source->getChapterList(dbManga);
            int chapterOrder = 0;
            for (const SChapter& sChap : sChapters) {
                Chapter chapter(
                    -1, // id (will be assigned by DB)
                    mangaIdToOpen,
                    sChap.url(),
                    sChap.name(),
                    sChap.scanlator(),
                    false, false, 0, // read, bookmark, lastPageRead
                    sChap.chapterNumber(),
                    chapterOrder++,
                    QDateTime::currentSecsSinceEpoch(), // dateFetch
                    sChap.dateUpload(),
                    QDateTime::currentSecsSinceEpoch() // lastModifiedAt
                );
                chapterRepo.insertChapter(chapter);
            }
            qDebug() << "Inserted" << sChapters.size() << "chapters for manga ID:" << mangaIdToOpen;
        } else {
            qWarning() << "Failed to insert new manga into database.";
            return;
        }
    } else {
        qDebug() << "Manga already in DB with ID:" << mangaIdToOpen;

        // Sync chapters anyway to catch new local files/updates
        QList<SChapter> sChapters = source->getChapterList(dbManga);
        QList<Chapter> existingChapters = chapterRepo.getChaptersByMangaId(mangaIdToOpen);

        // Simple sync: add only those that don't exist by URL
        for (const SChapter& sChap : sChapters) {
            bool exists = false;
            for (const Chapter& existing : existingChapters) {
                if (existing.url() == sChap.url()) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                Chapter chapter(
                    -1, mangaIdToOpen, sChap.url(), sChap.name(), sChap.scanlator(),
                    false, false, 0, sChap.chapterNumber(),
                    existingChapters.size(), // Use current size as order
                    QDateTime::currentSecsSinceEpoch(), sChap.dateUpload(),
                    QDateTime::currentSecsSinceEpoch()
                );
                chapterRepo.insertChapter(chapter);
            }
        }
    }

    if (mangaIdToOpen != -1) {
        // Re-fetch fresh manga object from DB to ensure we have ID and latest state
        Manga finalManga = mangaRepo.getMangaById(mangaIdToOpen);
        showMangaDetails(finalManga);

        // Refresh library in case it was added (even if not favorited yet,
        // though LibraryView filters by favorites, it's good practice)
        m_libraryView->refreshLibrary();
    }
}

void MainWindow::onChapterSelected(const Manga& manga, const Chapter& chapter)
{
    showReader(manga.id(), chapter.id());
}

void MainWindow::onNavigationRequested(int index)
{
    // Indexes:
    // 0: Library
    // 1: Updates
    // 2: History
    // 3: Browse
    // 4: Settings

    int stackIndex = -1;
    switch (index) {
        case 0: stackIndex = 0; break; // Library
        case 1: stackIndex = 1; break; // Updates
        case 2: stackIndex = 2; break; // History
        case 3: stackIndex = 3; break; // Browse
        case 4: stackIndex = 4; break; // Settings
    }

    if (stackIndex != -1) {
        if (stackIndex == 0) {
            m_libraryView->refreshLibrary();
        }

        // Don't track Details (5) as a "base" navigation point
        if (m_contentStack->currentIndex() != 5) {
            m_lastContentIndex = m_contentStack->currentIndex();
        }

        m_contentStack->setCurrentIndex(stackIndex);
    }
}

void MainWindow::onBackRequested()
{
    if (m_rootStack->currentWidget() == m_readerWidget) {
        // Back from Reader -> Dashboard (shows last content)
        m_rootStack->setCurrentWidget(m_dashboardWidget);
    } else {
        // We are in Dashboard. Check Content Stack.
        QWidget* currentContent = m_contentStack->currentWidget();

        if (currentContent == m_mangaDetailsView) {
            // Back from Details -> where we came from (Library or Browse)
            m_contentStack->setCurrentIndex(m_lastContentIndex);
        } else if (currentContent == m_browseStack) {
            // Inside Browse: Handle Browse Stack back navigation
            if (m_browseStack->currentWidget() == m_sourceBrowseView) {
                showSourceList();
            } else {
                // If at root of browse (Source List), maybe go to Library?
                // Standard Android behavior: Back at root tab -> Exit app (or go to default tab)
                // For Desktop, maybe do nothing.
            }
        }
        // Else: Updates, History, Settings, Library -> Do nothing (or exit app logic)
    }
}

void MainWindow::showMangaDetails(const Manga& manga)
{
    if (m_contentStack->currentIndex() != 5) {
        m_lastContentIndex = m_contentStack->currentIndex();
    }
    m_mangaDetailsView->setManga(manga);
    m_contentStack->setCurrentWidget(m_mangaDetailsView);
}

void MainWindow::showSourceList()
{
    m_sourceListView->refreshSources();
    m_browseStack->setCurrentWidget(m_sourceListView);
}

void MainWindow::showMangaList(long sourceId)
{
    m_currentSourceId = sourceId; // Store current source
    SourceBase* source = m_sourceManager->getSourceById(sourceId);
    if (source) {
        m_sourceBrowseView->setSource(source);
        m_browseStack->setCurrentWidget(m_sourceBrowseView);
    } else {
        qWarning() << "Source with ID" << sourceId << "not found.";
        showSourceList(); // Fallback to source list
    }
}

void MainWindow::showReader(long mangaId, long chapterId)
{
    m_rootStack->setCurrentWidget(m_readerWidget);
    m_readerWidget->loadChapter(mangaId, chapterId);
}
void MainWindow::onLocalMangaPathChanged(const QString& newPath)
{
    qDebug() << "Local manga path changed to:" << newPath;
    SourceBase* source = m_sourceManager->getSourceById(LocalSource::ID);
    if (source) {
        LocalSource* localSource = static_cast<LocalSource*>(source);
        localSource->setBaseDirectory(newPath);
        m_libraryView->refreshLibrary();

        // Use m_currentSourceId to check if we should refresh the browse view
        if (m_currentSourceId == LocalSource::ID) {
             showMangaList(LocalSource::ID);
        }
    }
}

