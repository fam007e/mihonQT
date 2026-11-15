#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

namespace AppVersion {
    constexpr const char* VERSION = "0.3.0-alpha";
    constexpr int VERSION_CODE = 300;

    inline QString getChangelog() {
        return QStringLiteral(
            "### 🔗 Tracking Services Integration\n"
            "- Connect your **AniList**, **MyAnimeList (MAL)**, and **MangaUpdates (Baka-Updates)** accounts.\n"
            "- Automatic chapter read synchronization upon finishing a chapter or batch marking.\n"
            "- Pre-fills current library reading progress if tracker is unread.\n"
            "- Rating & score synchronization (★ 0.0 - 10.0 in 0.5 step increments).\n"
            "- Official branded vector SVG badges for all supported trackers.\n\n"
            "### 📑 Multi-Chapter Selection & Batch Actions\n"
            "- Range selection with **Shift + Click** across chapter rows and checkboxes.\n"
            "- One-click **\"Mark previous chapters as read\"** in the chapter context menu (marks earlier chapters while keeping your target chapter unread so you can read it immediately).\n"
            "- Dimmed read chapter styling (38% opacity) and dynamic unread indicator hiding.\n\n"
            "### 🔒 Zero-Secret Build System\n"
            "- Build-time OAuth Client ID injection via CMake `AppSecrets.h` with zero credentials stored in Git.\n\n"
            "### 📦 Comprehensive Release Packaging\n"
            "- Automated multi-platform builds: Linux AppImage, Debian `.deb`, RPM, Arch Linux `.zst`, Windows `.zip`, and macOS `.dmg`."
        );
    }
}

#endif // APPVERSION_H
