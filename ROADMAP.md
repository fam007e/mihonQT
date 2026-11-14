# Roadmap: Porting Mihon to Qt for Desktop (macOS, Linux, Windows)

This document outlines the progress of porting Mihon to a cross-platform desktop application using Qt 6. Last updated: 2026-11-14, after the v0.1.0-alpha release.

## 🏁 Current Status: Alpha
All core logic and the main UI are implemented and functional. The first tagged release (v0.1.0-alpha) ships prebuilt binaries for Linux, Windows and macOS.

---

## ✅ Phase 1: Core Logic & MVP Reader
**Status: Completed**
- [x] Project Setup (CMake, Qt 6)
- [x] Database Layer (SQLite, Repositories)
- [x] Preferences (PreferenceManager)
- [x] Local Source (CBZ and Directory support)
- [x] MVP Reader (Webtoon and Paged modes)

## ✅ Phase 2: Library & UI Polish
**Status: Completed**
- [x] Library Screen (grid view with unread/download badges, category tabs, search and filters)
- [x] Manga Details Screen (Mihon-style header, chapter rows, chapter filter menu)
- [x] Improved Reader (tap zones, keyboard shortcuts)
- [x] Double-Page Spread (cover offset, wide-page detection, dynamic stepping)
- [x] Material 3 Theming (Mihon Default, Nord, Catppuccin, Tokyo Night, Dracula + AMOLED)
- [x] Full Keyboard Navigation (rail hub, arrows, Esc back, Ctrl+Q)

## 🚧 Phase 3: Online Sources & Networking
**Status: In Progress**
- [x] Networking Stack (NetworkAccessManager, worker-thread execution)
- [x] JavaScript Extension Engine (QJSEngine)
- [x] HTML5 Parsing (Gumbo with CSS selector engine)
- [x] Download Manager (multi-threaded, cancellation, binary caching of covers)
- [x] Async Browsing UI (non-blocking fetches with progress)
- [ ] Advanced Online Source Features (filtering, search categories)
- [ ] Batch extension install / update flow

## 🚧 Phase 4: Advanced Features & Deployment
**Status: In Progress**
- [x] Extension Repository System (external repo support; default repo is the generated `repo-qt` branch of [fam007e/extensions-source](https://github.com/fam007e/extensions-source/tree/repo-qt) — JS sources + index.min.json built by CI, seeded with WeebCentral, Toonily, ManhuaUS, HiperDex, Webtoons.com, MangaDex)
- [x] Statistics Dashboard
- [x] Backup & Restore
- [x] CI/CD Pipeline (Windows, Linux, macOS matrix)
- [x] Release Pipeline (tagged builds with per-platform artifacts — v0.1.0-alpha)
- [x] Documentation & FAQ (site + wiki)
- [x] Code Scanning (CodeQL, Flawfinder) and libFuzzer fuzzing
- [ ] Tracking Services (AniList, MyAnimeList — service code exists, UI wiring pending)
- [ ] Full Internationalization (i18n)
- [ ] Auto-Updater for Desktop
- [ ] Reader color filters and custom brightness presets
