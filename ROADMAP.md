# Roadmap: Porting Mihon to Qt for Desktop (macOS, Linux, Windows)

This document outlines the progress of porting Mihon to a cross-platform desktop application using Qt 6.

## 🏁 Current Status: Alpha
Most core logic and essential UI components are implemented and functional.

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
- [x] Library Screen (Grid/List views)
- [x] Manga Details Screen
- [x] Improved Reader (Tap zones, basic shortcuts)
- [x] Theming System (Nord, Catppuccin, etc.)

## 🚧 Phase 3: Online Sources & Networking
**Status: In Progress**
- [x] Networking Stack (NetworkAccessManager)
- [x] JavaScript Extension Engine
- [x] Download Manager (Multi-threaded, Cancellation support)
- [ ] Advanced Online Source Features (Filtering, Search categories)

## 🚧 Phase 4: Advanced Features & Deployment
**Status: In Progress**
- [x] Extension Repository System (External Repo Support)
- [x] Statistics Dashboard
- [x] Backup & Restore
- [x] CI/CD Pipeline (Windows, Linux, macOS Matrix)
- [x] Documentation & FAQ
- [ ] Tracking Services (AniList, MyAnimeList)
- [ ] Full Internationalization (i18n)
- [ ] Auto-Updater for Desktop
