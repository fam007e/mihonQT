#include "MainWindow.h"
#include "database/DatabaseManager.h"
#include "database/MangaRepository.h"
#include "database/ChapterRepository.h"
#include "model/Manga.h"
#include "model/Chapter.h"
#include "source/LocalSource.h" // Needed for dummy data
#include "ui/SourceListView.h"  // Include for SourceListView
#include "ui/MangaListView.h"   // Include for MangaListView
#include "ui/ThemeManager.h"

#include <QSqlQuery> // Include QSqlQuery
#include <QSqlError> // Include QSqlError
#include <QMenuBar>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout> // Still needed for main window layout if any
#include <QSettings> // For storing settings like local manga path
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_sourceManager(new SourceManager(this))
    , m_mainStack(new QStackedWidget(this)) // Main stack: Tabs, Details, Reader
    , m_jsEngine(new QJSEngine(this))
    , m_networkManager(new NetworkAccessManager(m_jsEngine, this))
{
    // Database setup
    m_database = DatabaseManager::instance().database();
    if (m_database.isOpen()) {
        qDebug() << "MainWindow: Database connection established.";
    } else {
        qDebug() << "MainWindow: Failed to establish database connection.";
        QMessageBox::critical(this, "Database Error", "Failed to establish database connection.");
    }

    // --- Initialize Components ---

    // 1. Library View
    m_libraryView = new LibraryView(this);
    connect(m_libraryView, &LibraryView::mangaSelected, this, &MainWindow::onMangaSelected);

    // 2. Browse View (Source List + Manga List)
    m_browseStack = new QStackedWidget(this);
    m_sourceListView = new SourceListView(m_sourceManager, this);
    m_mangaListView = new MangaListView(this);
    
    m_browseStack->addWidget(m_sourceListView);
    m_browseStack->addWidget(m_mangaListView);
    
    connect(m_sourceListView, &SourceListView::sourceSelected, this, &MainWindow::onSourceSelected);
    connect(m_mangaListView, &MangaListView::mangaSelected, this, &MainWindow::onMangaSelected);

    // 3. Main Tabs (Library + Browse)
    m_mainTabWidget = new QTabWidget(this);
    m_mainTabWidget->addTab(m_libraryView, "Library");
    m_mainTabWidget->addTab(m_browseStack, "Browse");
    
    // Refresh library when tab is selected
    connect(m_mainTabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 0) { // Library tab
            m_libraryView->refreshLibrary();
        }
    });

    // 4. Details View
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

    // --- Setup Main Stack ---
    // --- Setup Main Stack ---
    m_mainStack->addWidget(m_mainTabWidget);      // Index 0: Tabs
    m_mainStack->addWidget(m_mangaDetailsView);   // Index 1: Details
    m_mainStack->addWidget(m_readerWidget);       // Index 2: Reader
    m_mainStack->addWidget(m_settingsView);       // Index 3: Settings

    setCentralWidget(m_mainStack);

    // Initialize Sources
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

    setupUi();
    
    // Initial refresh
    m_libraryView->refreshLibrary();
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
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolBar->addWidget(spacer);

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
        m_mainStack->setCurrentWidget(m_settingsView);
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

    MangaRepository mangaRepo(m_database);
    ChapterRepository chapterRepo(m_database);
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
        qDebug() << "Manga already in library with ID:" << mangaIdToOpen;
        // Optionally, we can update chapters here as well (a "sync" operation)
    }

    if (mangaIdToOpen != -1) {
        // Re-fetch fresh manga object from DB to ensure we have ID and latest state
        Manga finalManga = mangaRepo.getMangaById(mangaIdToOpen);
        showMangaDetails(finalManga);
    }
}

void MainWindow::onChapterSelected(const Manga& manga, const Chapter& chapter)
{
    showReader(manga.id(), chapter.id());
}

void MainWindow::onBackRequested()
{
    if (m_mainStack->currentWidget() == m_readerWidget) {
        // Back from Reader -> Details
        m_mainStack->setCurrentWidget(m_mangaDetailsView);
        // Refresh details in case chapter progress changed
        // m_mangaDetailsView->refreshChapters(); 
    } else if (m_mainStack->currentWidget() == m_mangaDetailsView) {
        // Back from Details -> Tabs
        m_mainStack->setCurrentWidget(m_mainTabWidget);
    } else if (m_mainStack->currentWidget() == m_settingsView) {
        // Back from Settings -> Tabs (or previous view, but Tabs is safe default)
        m_mainStack->setCurrentWidget(m_mainTabWidget);
    } else if (m_mainStack->currentWidget() == m_mainTabWidget) {
        // Inside Tabs: Handle Browse Stack back navigation
        if (m_mainTabWidget->currentWidget() == m_browseStack) {
            if (m_browseStack->currentWidget() == m_mangaListView) {
                showSourceList();
            }
        }
    }
}

void MainWindow::showMangaDetails(const Manga& manga)
{
    m_mangaDetailsView->setManga(manga);
    m_mainStack->setCurrentWidget(m_mangaDetailsView);
}

void MainWindow::showSourceList()
{
    m_browseStack->setCurrentWidget(m_sourceListView);
    // setWindowTitle("MihonQT - Sources"); // Title managed by tabs now?
}

void MainWindow::showMangaList(long sourceId)
{
    SourceBase* source = m_sourceManager->getSourceById(sourceId);
    if (source) {
        m_mangaListView->populateManga(source->getPopularManga()); // For now, show popular manga
        // setWindowTitle("MihonQT - " + source->name());
        m_browseStack->setCurrentWidget(m_mangaListView);
    } else {
        qWarning() << "Source with ID" << sourceId << "not found.";
        showSourceList(); // Fallback to source list
    }
}

void MainWindow::showReader(long mangaId, long chapterId)
{
    m_mainStack->setCurrentWidget(m_readerWidget);
    m_readerWidget->loadChapter(mangaId, chapterId);
}