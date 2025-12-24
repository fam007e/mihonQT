#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QStackedWidget> // For managing different views
#include <QJSEngine> // Include QJSEngine
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QMenu>

#include "ui/reader/ReaderWidget.h"
#include "source/SourceManager.h" // Include SourceManager
#include "source/LocalSource.h"   // Include LocalSource
#include "network/NetworkAccessManager.h" // Include NetworkAccessManager
#include "source/JavascriptSource.h" // Include JavascriptSource
#include "model/Manga.h" // Include for signal/slot
#include "model/Chapter.h" // [NEW]
#include "ui/SourceListView.h"
#include "ui/SourceBrowseView.h"
#include "ui/MangaDetailsView.h"
#include "ui/SettingsView.h"

// Forward declarations for new UI components
class SidebarWidget;
class UpdatesView;
class HistoryView;
class LibraryView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSourceSelected(long sourceId); // Slot for when a source is selected
    void onMangaSelected(const Manga& manga);   // Slot for when a manga is selected
    void onChapterSelected(const Manga& manga, const Chapter& chapter); // [NEW]
    void onBackRequested(); // [NEW]
    void onLocalMangaPathChanged(const QString& newPath);
    void onNavigationRequested(int index);

private:
    void setupUi();
    void showSourceList(); // New method to show the source list view
    void showMangaList(long sourceId); // New method to show manga list for a source
    void showReader(long mangaId, long chapterId);
    void showMangaDetails(const Manga& manga); // [NEW]

    // QSqlDatabase m_database; // Removed member
    SourceManager *m_sourceManager;
    int m_lastContentIndex = 0;
    long m_currentSourceId = -1; // [NEW] Track current source

    // Navigation
    QTabWidget *m_mainTabWidget; // [NEW] Top-level tabs (Library, Browse)

    // Browse Tab Stack
    QJSEngine *m_jsEngine;
    NetworkAccessManager *m_networkManager;

    // UI Structure
    QStackedWidget *m_rootStack;      // Level 1: [Dashboard, Reader]

    // Dashboard (Level 2)
    QWidget *m_dashboardWidget;
    SidebarWidget *m_sidebar;
    QStackedWidget *m_contentStack;   // Level 3: [Library, Updates, History, Browse, Details, Settings]

    // Views
    LibraryView *m_libraryView;
    UpdatesView *m_updatesView;
    HistoryView *m_historyView;

    QStackedWidget *m_browseStack;
    SourceListView *m_sourceListView;
    SourceBrowseView *m_sourceBrowseView;

    MangaDetailsView *m_mangaDetailsView;
    SettingsView *m_settingsView;

    ReaderWidget *m_readerWidget;

    // Main Window Widgets
    QToolBar *m_toolBar;
    QAction *m_hamburgerAction;
    QMenu *m_hamburgerMenu;

    // Menu Actions
    QAction *m_downloadedOnlyAction;
    QAction *m_incognitoModeAction;
    QAction *m_downloadQueueAction;
    QAction *m_categoriesAction;
    QAction *m_statsAction;
    QAction *m_settingsAction;
    QAction *m_aboutAction;
};

#endif // MAINWINDOW_H
