# Roadmap: Porting Mihon to Qt for Desktop (macOS, Linux, Windows)

This document outlines a high-level roadmap for porting the core functionality of the Android-based Mihon application to a cross-platform desktop application using the Qt framework.

## Core Philosophy

The key to a successful port is leveraging Mihon's existing modular architecture. The `domain`, `data`, and much of the `source` logic are written in Kotlin and are largely platform-agnostic. The primary challenge is to replace the Android-specific UI and platform integrations with Qt/C++ alternatives.

We will follow a phased approach, starting with a Minimum Viable Product (MVP) and gradually adding features.

---

## Phase 1: Core Logic & MVP Reader

**Goal:** Create a basic, functional desktop application that can read a local manga file (CBZ or directory). This phase focuses on setting up the project and porting the essential, non-UI backend components.

1.  **Project Setup:**
    *   Initialize a new C++ project using CMake.
    *   Set up the Qt 6 framework, including Core, GUI, Widgets, and Network modules.
    *   Create a basic `QMainWindow` as the main application window.

2.  **Porting Core Data Layer:**
    *   **Database:**
        *   Integrate SQLite using Qt's `QtSql` module.
        *   Re-create the database schema from Mihon's `.sq` files (`mangas.sq`, `chapters.sq`, etc.) in a new `schema.sql` file.
        *   Write C++ data access objects (DAOs) or repositories to replace the SQLDelight-generated queries. These C++ classes will handle all `INSERT`, `UPDATE`, `SELECT`, and `DELETE` operations.
    *   **Preferences:**
        *   Implement a `PreferenceManager` class in C++ using `QSettings` to replace Android's `SharedPreferences`. This class will manage all application settings.

3.  **Implement the Local Source:**
    *   Create a `LocalSource` class in C++ that implements the logic from Mihon's `source-local` module.
    *   Use `QDir` and `QFile` to scan directories for manga, read chapter files, and extract metadata (`ComicInfo.xml`).
    *   This class will be responsible for populating the database with local manga.

4.  **Develop the MVP Reader UI:**
    *   **Reader Widget:** Create a `ReaderWidget` in C++.
    *   **Viewer:** Implement a basic `WebtoonViewer` using `QScrollArea` and a `QVBoxLayout` to display chapter pages vertically. This is simpler to implement initially than a pager.
    *   **Page Display:** Use `QLabel` with `QPixmap` to display images. Handle image loading and decoding in a background thread using `QThread` or `QtConcurrent` to avoid freezing the UI.
    *   **Basic Controls:** Implement basic navigation (scrolling) and a button to open a local manga folder.

---

## Phase 2: Library & UI Polish

**Goal:** Build out the library UI and improve the reader experience.

1.  **Library UI:**
    *   **Library Screen:** Create a `LibraryWidget` to display all manga from the database.
    *   **Manga Grid/List:** Use `QListWidget` or `QTableWidget` to display manga covers and titles.
    *   **Data Binding:** Connect the `LibraryWidget` to the C++ database repositories to display and update library content.

2.  **Manga Details Screen:**
    *   Create a `MangaDetailsWidget` that displays manga information (cover, description, author) and a list of chapters.
    *   Add a "Start Reading" button that opens the `ReaderWidget` for a selected chapter.

3.  **Improved Reader:**
    *   **Pager Viewer:** Implement a `PagerViewer` using `QStackedWidget` or a similar widget to display one or two pages at a time.
    *   **Navigation:** Add tap zones and keyboard shortcuts (e.g., arrow keys) for page navigation.
    *   **Reader Settings:** Implement a basic settings dialog for the reader (e.g., changing background color).

---

## Phase 3: Online Sources & Networking

**Goal:** Add support for fetching manga from online sources. This is a significant step towards feature parity with the Android app.

1.  **Networking Stack:**
    *   Implement a `NetworkManager` class in C++ using `QNetworkAccessManager` to handle all HTTP requests.
    *   Integrate a JavaScript engine (like Qt's `QJSEngine`) to handle potential web challenges. This will be a replacement for the `WebView`-based Cloudflare interceptor. **Note:** Bypassing Cloudflare on desktop without a full browser engine is significantly more complex than on Android and may require dedicated libraries.

2.  **Porting an Online Source:**
    *   Choose a single, simple `ParsedHttpSource` from the Mihon ecosystem to port.
    *   Re-write the source's parsing logic in C++ using a C++ HTML parsing library (e.g., `libxml2`, `Gumbo-parser`, or a Qt-based one).
    *   This involves translating the CSS selectors and data extraction logic from Kotlin to C++.

3.  **Downloads:**
    *   Implement a `DownloadManager` class to download chapter images using `QNetworkAccessManager`.
    *   Save downloaded chapters to a local directory, structured in a way that the `LocalSource` can read.

---

## Phase 4: Extensions & Advanced Features

**Goal:** Re-introduce Mihon's extensibility and add other advanced features.

1.  **Extension System Architecture:**
    *   **Design a Plugin System:** This is the most complex task. The Android APK-based extension system cannot be ported directly. A new plugin architecture must be designed for desktop.
    *   **Option A (Shared Libraries):** Extensions could be compiled as native shared libraries (`.dll`, `.so`, `.dylib`). This would likely require extension developers to write in C++.
    *   **Option B (Inter-Process Communication or Embedded VM):** A more flexible approach would be to run the original Kotlin extensions in a sandboxed JVM or a GraalVM native image. The Qt application would communicate with these extensions via IPC. This preserves the existing extension ecosystem but adds significant architectural complexity.

2.  **Implement Core Features:**
    *   **Tracking:** Add support for tracking services like AniList and MyAnimeList by implementing their APIs using `QNetworkAccessManager`.
    *   **Advanced Filtering:** Implement the full filtering and sorting system for the library and sources.
    *   **Data Sync:** Implement backup and restore functionality.

3.  **UI/UX Refinements:**
    *   Polish the entire UI, ensuring it feels native on each platform (macOS, Linux, Windows).
    *   Implement all remaining settings from the Android app.
    *   Add multi-language support using Qt's internationalization tools.

This roadmap provides a structured path from a basic local reader to a full-featured, cross-platform desktop application with online capabilities and extensibility.
