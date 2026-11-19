#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

namespace AppVersion {
    constexpr const char* VERSION = "0.4.0-alpha";
    constexpr int VERSION_CODE = 400;

    inline QString getChangelog() {
        return QStringLiteral(
            "### 🎨 Native Widget Style Support\n"
            "- **Settings → Appearance → Widget Style**: run the app with your desktop's own Qt style — System default, Breeze, Oxygen, Kvantum, Fusion, or any installed style.\n"
            "- Dropdowns stay readable even under translucent Kvantum themes.\n\n"
            "### 📊 Statistics Dashboard\n"
            "- Four sections: **Library** (manga, chapters, read, unread), **Reading state** (started, completed, categories), **Activity** (chapters read in the last 7 and 30 days, last-read date), and **Outside library** (browsed-only mangas and their chapters).\n"
            "- Counts are library-scoped: content synced while browsing but never added no longer inflates the numbers.\n\n"
            "### 📖 Reader\n"
            "- **W** cycles reading modes (Webtoon → Right-to-Left → Left-to-Right) and leaves double-page mode to Webtoon.\n"
            "- **Shift+D / Ctrl+D** toggles Double Page Spread — always manga-ordered (current page on the right, next on the left) from any reading mode.\n"
            "- Double-page is remembered per chapter: close mid-chapter and reopening resumes in double-page; a new chapter starts in your base reading mode again.\n"
            "- The reading direction decides which arrow key moves forward in the page count (Right in left-to-right, Left in right-to-left) — keys and tap zones, single or double page.\n\n"
            "### 🛠 Fixes\n"
            "- Chapters ending on a spread are now marked as read.\n"
            "- **Mark as unread** resets the saved page — a read→unread chapter reopens at page 1.\n"
            "- The manga details list refreshes instantly when you come back from the reader.\n"
            "- Selected chapter highlight no longer bleeds past its rounded corners."
        );
    }
}

#endif // APPVERSION_H
