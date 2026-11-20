<p align="center">
  <img src="logo.png" width="250" height="250" alt="MihonQT Logo">
</p>

# MihonQT Documentation

Welcome to the official documentation for MihonQT, a cross-platform desktop manga reader.

## 📖 Available Documentation

- **[Building](wiki/Building.html)** - Per-platform dependency lists and CMake invocations.
- **[Keyboard Shortcuts](wiki/Keyboard-Shortcuts.html)** - The full keyboard navigation map.
- **[JavaScript Extensions](wiki/Extensions.html)** - Writing manga sources for MihonQT.
- **[Themes](wiki/Themes.html)** - Shipped Material 3 themes and the token system.
- **[Frequently Asked Questions (FAQ)](FAQ.html)** - Common questions and troubleshooting tips.
- **[Support / Donate](DONATE.html)** - How to support MihonQT and Mihon.
- **[Security Audit](SECURITY_AUDIT.html)** - Detailed analysis of the project's security posture.
- **[Static Analysis](CPPCHECK.html)** - CppCheck configuration and usage.
- **[GitHub Wiki](https://github.com/fam007e/mihonQT/wiki)** - Mirror of these pages.
- **[GitHub Repository](https://github.com/fam007e/mihonQT)** - Source code and issue tracking.

## 🚀 About the Project

MihonQT aims to bring the best features of the Mihon Android app to the desktop, providing a fast, extensible, and privacy-focused reading experience.

### Key Features
- **Material 3 Inspired UI:** A modern interface consistent with Tachiyomi/Mihon Android.
- **Double-Page Spread Reader:** Always manga-ordered spreads (current page on the right, next on the left) with cover offset and wide-page detection; RTL/LTR/webtoon modes where the reading direction decides which arrow key moves forward.
- **Native Widget Styles:** Run the app with your desktop's own Qt style (Breeze, Kvantum, Fusion, System default) or the built-in Material look.
- **Statistics Dashboard:** Library overview (read/unread), reading state (started/completed), recent activity (last 7/30 days) and outside-library tracking of browsed content.
- **Full Keyboard Navigation:** Rail-focused hub-and-spoke navigation with Esc/Enter/arrows.
- **Tracking Services Integration:** Full OAuth sync for **AniList**, **MyAnimeList (MAL)**, and **MangaUpdates (Baka-Updates)** with automatic progress update, ratings (`★ 0.0 - 10.0`), and library pre-fill.
- **Batch Chapter Management:** Range selection with <kbd>Shift</kbd> + Click, one-click "Mark previous chapters as read" (keeps target chapter unread so you can read it immediately), and bulk download/bookmarking.
- **JavaScript Extensions:** HTML5-correct parsing (Gumbo) with CSS selectors, run off the UI thread.
- **Dynamic Theming:** The Mihon Material 3 palettes (Mihon Default, Nord, Catppuccin, Tokyo Night, Dracula) plus AMOLED.
- **Multi-threaded Download Manager:** Fast and robust chapter downloads for offline reading.
- **Global Security Policies:** Enforce HTTPS and restrict untrusted extension network access.
- **Incognito Reading Mode:** Privacy-first reading with a single toggle.

## 🔒 Security & Privacy

The current state of MihonQT incorporates all major mitigations from the 2026 Security Audit, making it one of the most secure desktop manga readers available. Learn more in our **[Security Audit](SECURITY_AUDIT.md)**.
