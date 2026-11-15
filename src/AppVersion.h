#ifndef APPVERSION_H
#define APPVERSION_H

#include <QString>

namespace AppVersion {
    constexpr const char* VERSION = "0.1.0-alpha";
    constexpr int VERSION_CODE = 100;

    inline QString getChangelog() {
        return QStringLiteral(
            "### 📖 Per-Manga Viewing Mode\n"
            "- Reading modes (Webtoon, Left-to-Right, Right-to-Left, Double-Page Spread) are now remembered individually per manga.\n"
            "- Changing a mode while reading no longer alters the global reader default.\n\n"
            "### ⌨️ Instant Double-Page Spread Hotkey\n"
            "- Press **W**, **Shift+D**, or **Ctrl+D** anytime while reading to toggle Double Page Spread mode on and off instantly.\n\n"
            "### 🎨 Material 3 Layout & Typography\n"
            "- Dynamic 2-line word wrapping on browse grid cards with font-metric elision.\n"
            "- Automatic HTML entity decoding and paragraph formatting for manga synopses.\n"
            "- Stable, auto-centering Reader Settings menu with rounded M3 dropdowns.\n\n"
            "### 📄 Standardized Apache-2.0 License\n"
            "- Project licensing harmonized with upstream Mihon and Keiyoushi extensions.\n"
            "- Responsive in-app license reflow across any desktop window size.\n\n"
            "### 🔒 Extension & Network Hardening\n"
            "- Cookie jar synchronization across WebViews and image loaders.\n"
            "- Support for custom HTTP headers and API tokens (`Network.setHeader`).\n"
            "- External community extensions repository (`repo-qt`) with trust isolation."
        );
    }
}

#endif // APPVERSION_H
