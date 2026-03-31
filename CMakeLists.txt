cmake_minimum_required(VERSION 3.16)
project(MihonQT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Qt Setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Sql Concurrent Quick Qml Core5Compat WebEngineWidgets WebEngineCore REQUIRED) # Add Core5Compat and WebEngine

# --- QuaZip ---
set(QUAZIP_QT_MAJOR_VERSION 6 CACHE STRING "Qt version to use (6 or 5)" FORCE)
set(QUAZIP_ENABLE_QTEXTCODEC OFF CACHE BOOL "Enable use of QTextCodec" FORCE) # Disable QTextCodec
add_subdirectory(libs/quazip)

# --- Project Files ---
add_executable(MihonQT
    src/main.cpp
    src/MainWindow.cpp
    src/database/DatabaseManager.cpp
    src/model/Manga.cpp
    src/model/Chapter.cpp
    src/source/model/SChapter.cpp
    src/database/MangaRepository.cpp
    src/database/ChapterRepository.cpp
    src/config/PreferenceManager.cpp
    src/source/LocalSource.cpp
    src/source/SourceBase.cpp
    src/source/SourceManager.cpp
    src/source/JavascriptSource.cpp # Add JavascriptSource.cpp
    src/source/HtmlParser.cpp # Add HtmlParser.cpp
    src/network/NetworkAccessManager.cpp # Add NetworkAccessManager.cpp
    src/ui/reader/ReaderWidget.cpp
    src/ui/reader/ReaderSettingsOverlay.cpp
    src/ui/SourceListView.cpp
    src/ui/MangaListView.cpp
    src/ui/MangaDetailsView.cpp
    src/ui/ThemeManager.cpp
    src/ui/SettingsView.cpp
    src/ui/CategoryEditDialog.cpp
    src/ui/SidebarWidget.cpp
    src/ui/MoreView.cpp
    src/ui/WebViewDialog.cpp
    src/ui/UpdatesView.cpp
    src/ui/HistoryView.cpp
    src/ui/LibraryView.cpp
    src/ui/SourceBrowseView.cpp
    src/ui/MangaCoverCard.cpp
    src/ui/DownloadView.cpp
    src/download/DownloadManager.cpp
    src/tracking/TrackingService.cpp
    src/tracking/AniListTracker.cpp
    src/ui/TrackingWidget.cpp
    src/backup/BackupManager.cpp
    src/model/Category.cpp
    src/database/CategoryRepository.cpp
    src/database/HistoryRepository.cpp
    src/source/ExtensionManager.cpp
    src/source/ExtensionRepoManager.cpp
    src/ui/ExtensionsView.cpp
    src/ui/ExtensionManagerView.cpp
    src/ui/StatisticsView.cpp
    resources.qrc
)

# --- Link Libraries ---
target_link_libraries(MihonQT PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent Qt6::Quick Qt6::Qml Qt6::WebEngineWidgets Qt6::WebEngineCore QuaZip::QuaZip) # Link QuaZip and WebEngine

# --- Deployment ---
if(WIN32)
    include(cmake/WinDeploy.cmake)
    deploy_windows_dependencies(MihonQT USES_QT)
endif()

# --- Include Directories ---
target_include_directories(MihonQT PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/source
    ${CMAKE_CURRENT_SOURCE_DIR}/src/source/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network # Add src/network include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/reader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/download
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tracking
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backup
)
