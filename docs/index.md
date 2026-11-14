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
- **[Security Audit](SECURITY_AUDIT.html)** - Detailed analysis of the project's security posture.
- **[Static Analysis](CPPCHECK.html)** - CppCheck configuration and usage.
- **[GitHub Wiki](https://github.com/fam007e/mihonQT/wiki)** - Mirror of these pages.
- **[GitHub Repository](https://github.com/fam007e/mihonQT)** - Source code and issue tracking.

## 🚀 About the Project

MihonQT aims to bring the best features of the Mihon Android app to the desktop, providing a fast, extensible, and privacy-focused reading experience.

### Key Features
- **Material 3 Inspired UI:** A modern interface consistent with Tachiyomi/Mihon Android.
- **Double-Page Spread Reader:** Cover offset, wide-page detection, RTL/LTR/vertical modes.
- **Full Keyboard Navigation:** Rail-focused hub-and-spoke navigation with Esc/Enter/arrows.
- **JavaScript Extensions:** HTML5-correct parsing (Gumbo) with CSS selectors, run off the UI thread.
- **Dynamic Theming:** The Mihon Material 3 palettes (Mihon Default, Nord, Catppuccin, Tokyo Night, Dracula) plus AMOLED.
- **Multi-threaded Download Manager:** Fast and robust chapter downloads for offline reading.
- **Global Security Policies:** Enforce HTTPS and restrict untrusted extension network access.
- **Incognito Reading Mode:** Privacy-first reading with a single toggle.

## 🔒 Security & Privacy

The current state of MihonQT incorporates all major mitigations from the 2026 Security Audit, making it one of the most secure desktop manga readers available. Learn more in our **[Security Audit](SECURITY_AUDIT.md)**.
