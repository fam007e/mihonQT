cmake_minimum_required(VERSION 3.16)
project(MihonQT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Options ---
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(BUILD_FUZZERS "Build libFuzzer fuzzing targets" OFF)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled: AddressSanitizer and UndefinedBehaviorSanitizer")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

# --- Qt Setup ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Sql Concurrent Quick Qml Core5Compat WebEngineWidgets WebEngineCore REQUIRED) # Add Core5Compat and WebEngine

# --- Gumbo HTML5 Parser & WebP ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(GUMBO REQUIRED gumbo)
pkg_check_modules(WEBP REQUIRED libwebp)

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
    src/util/ImageUtil.cpp # WebP and Image utility
    src/util/CoverCache.cpp # Cover memory+disk cache
    src/network/NetworkAccessManager.cpp # Add NetworkAccessManager.cpp
    src/ui/reader/ReaderWidget.cpp
    src/ui/reader/ReaderSettingsOverlay.cpp
    src/ui/reader/ReaderBarsOverlay.cpp
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
target_link_libraries(MihonQT PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Concurrent Qt6::Quick Qt6::Qml Qt6::WebEngineWidgets Qt6::WebEngineCore QuaZip::QuaZip ${GUMBO_LIBRARIES} ${WEBP_LIBRARIES}) # Link QuaZip, WebEngine, Gumbo and WebP

# --- Deployment ---
if(WIN32)
    include(cmake/WinDeploy.cmake)
    deploy_windows_dependencies(MihonQT USES_QT)
endif()

# --- Include Directories ---
target_include_directories(MihonQT PRIVATE
    ${GUMBO_INCLUDE_DIRS}
    ${WEBP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/source
    ${CMAKE_CURRENT_SOURCE_DIR}/src/source/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network # Add src/network include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util # Add src/util include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/reader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/download
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tracking
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backup
)

# --- Unit Tests ---
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt6 COMPONENTS Test REQUIRED)

    function(add_mihon_test test_name test_source)
        add_executable(${test_name} ${test_source} ${ARGN})
        target_include_directories(${test_name} PRIVATE
            ${GUMBO_INCLUDE_DIRS}
            ${WEBP_INCLUDE_DIRS}
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/database
            ${CMAKE_CURRENT_SOURCE_DIR}/src/model
            ${CMAKE_CURRENT_SOURCE_DIR}/src/config
            ${CMAKE_CURRENT_SOURCE_DIR}/src/source
            ${CMAKE_CURRENT_SOURCE_DIR}/src/source/model
            ${CMAKE_CURRENT_SOURCE_DIR}/src/network
            ${CMAKE_CURRENT_SOURCE_DIR}/src/util
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
            ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/reader
            ${CMAKE_CURRENT_SOURCE_DIR}/src/download
            ${CMAKE_CURRENT_SOURCE_DIR}/src/tracking
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backup
        )
        target_link_libraries(${test_name} PRIVATE
            Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Test
            ${GUMBO_LIBRARIES} ${WEBP_LIBRARIES}
        )
        add_test(NAME ${test_name} COMMAND ${test_name})
    endfunction()

    add_mihon_test(test_html_parser tests/test_html_parser.cpp src/source/HtmlParser.cpp)
    add_mihon_test(test_chapter_sort tests/test_chapter_sort.cpp src/model/Chapter.cpp src/model/Manga.cpp)
    add_mihon_test(test_image_util tests/test_image_util.cpp src/util/ImageUtil.cpp)
    add_mihon_test(test_database tests/test_database.cpp
        src/database/DatabaseManager.cpp
        src/database/MangaRepository.cpp
        src/database/ChapterRepository.cpp
        src/database/CategoryRepository.cpp
        src/database/HistoryRepository.cpp
        src/model/Manga.cpp
        src/model/Chapter.cpp
        src/model/Category.cpp
        resources.qrc
    )
    add_mihon_test(test_spread_logic tests/test_spread_logic.cpp)
endif()

# --- Fuzzers (libFuzzer) ---
if(BUILD_FUZZERS)
    function(add_mihon_fuzzer fuzzer_name fuzzer_source)
        add_executable(${fuzzer_name} ${fuzzer_source} ${ARGN})
        target_compile_options(${fuzzer_name} PRIVATE -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -g)
        target_link_options(${fuzzer_name} PRIVATE -fsanitize=fuzzer,address,undefined)
        target_include_directories(${fuzzer_name} PRIVATE
            ${GUMBO_INCLUDE_DIRS}
            ${WEBP_INCLUDE_DIRS}
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/database
            ${CMAKE_CURRENT_SOURCE_DIR}/src/model
            ${CMAKE_CURRENT_SOURCE_DIR}/src/config
            ${CMAKE_CURRENT_SOURCE_DIR}/src/source
            ${CMAKE_CURRENT_SOURCE_DIR}/src/source/model
            ${CMAKE_CURRENT_SOURCE_DIR}/src/network
            ${CMAKE_CURRENT_SOURCE_DIR}/src/util
        )
        target_link_libraries(${fuzzer_name} PRIVATE
            Qt6::Core Qt6::Gui
            ${GUMBO_LIBRARIES} ${WEBP_LIBRARIES}
        )
    endfunction()

    add_mihon_fuzzer(fuzz_html_parser fuzz/fuzz_html_parser.cpp src/source/HtmlParser.cpp)
    add_mihon_fuzzer(fuzz_image_util fuzz/fuzz_image_util.cpp src/util/ImageUtil.cpp)
    add_mihon_fuzzer(fuzz_chapter_sort fuzz/fuzz_chapter_sort.cpp src/model/Chapter.cpp src/model/Manga.cpp)
endif()
