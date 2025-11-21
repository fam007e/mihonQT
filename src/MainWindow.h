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
#include "ui/SourceListView.h" // [NEW]
#include "ui/MangaListView.h" // [NEW]
#include "ui/LibraryView.h"      // [NEW]
#include "ui/MangaDetailsView.h" // [NEW]
#include "ui/SettingsView.h"     // [NEW]

// Forward declarations for new UI components
// class SourceListView; // No longer needed, now included
// class MangaListView; // No longer needed, now included

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

private:
    void setupUi();
    void showSourceList(); // New method to show the source list view
    void showMangaList(long sourceId); // New method to show manga list for a source
    void showReader(long mangaId, long chapterId);
    void showMangaDetails(const Manga& manga); // [NEW]

    QSqlDatabase m_database;
    SourceManager *m_sourceManager;
    
    // Navigation
    QTabWidget *m_mainTabWidget; // [NEW] Top-level tabs (Library, Browse)
    
    // Browse Tab Stack
    QStackedWidget *m_browseStack; 
    SourceListView *m_sourceListView;
    MangaListView *m_mangaListView;

    // Library Tab Stack (Optional, if we want details view inside library tab)
    // For simplicity, let's put DetailsView in both or use a global stack?
    // Better approach:
    // Tab 1: LibraryView
    // Tab 2: Browse (SourceList -> MangaList)
    // When a manga is selected in EITHER, we switch to a "Details" view?
    // Or we have a global QStackedWidget containing:
    // 1. MainTabs (Library, Browse)
    // 2. MangaDetails
    // 3. Reader
    
    QStackedWidget *m_mainStack; // Replaces m_stackedWidget as the central widget
    
    LibraryView *m_libraryView;
    MangaDetailsView *m_mangaDetailsView;
    ReaderWidget *m_readerWidget;
    SettingsView *m_settingsView;

    QJSEngine *m_jsEngine;
    NetworkAccessManager *m_networkManager;

    // Hamburger Menu
    QToolBar *m_toolBar;
    QAction *m_hamburgerAction;
    QMenu *m_hamburgerMenu;
    
    // Hamburger Menu Actions
    QAction *m_downloadedOnlyAction;
    QAction *m_incognitoModeAction;
    QAction *m_downloadQueueAction;
    QAction *m_categoriesAction;
    QAction *m_statsAction;
    QAction *m_settingsAction;
    QAction *m_aboutAction;
};

#endif // MAINWINDOW_H
