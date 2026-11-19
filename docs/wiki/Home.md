# Home

Welcome to the **MihonQT** wiki — the desktop port of the
[Mihon](https://github.com/mihonapp/mihon) manga reader, built with C++ and
Qt 6.

## Pages

- **[[Building]]** — per-platform dependency lists and CMake invocations
- **[[Keyboard-Shortcuts]]** — the full keyboard navigation map
- **[[Extensions]]** — writing JavaScript manga sources
- **[[Themes]]** — shipped Material 3 themes and the token system

## Quick start

```bash
# Linux (Debian/Ubuntu)
sudo apt install build-essential cmake pkg-config qt6-base-dev \
    qt6-webengine-dev qt6-declarative-dev qt6-5compat-dev \
    libgumbo-dev libwebp-dev
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/MihonQT
```

## Highlights

- **Double-page spread reader** — always manga-ordered (current page right,
  next left), with cover offset, wide-page detection, and a per-chapter
  memory: close mid-chapter and reopening resumes in double-page. Webtoon
  (long strip) stays single-column.
- **Native widget style support** — run the app with your desktop's own Qt
  style (Breeze, Oxygen, Kvantum, Fusion, System default) or the built-in
  Material look
- **JavaScript extensions** — full HTML5 parsing with CSS selectors, running
  off the UI thread
- **Mihon Android theming** — the same Material 3 palettes, plus AMOLED
- **Library management** — categories, unread/download badges, filters,
  download queue, reading history
- **Integrated Trackers** — native support for **AniList**, **MyAnimeList (MAL)**,
  and **MangaUpdates (Baka-Updates)** with score rating, progress auto-sync, and
  pre-filled chapter counts
- **Batch Chapter Management** — <kbd>Shift</kbd> + Click range selection,
  "Mark previous chapters as read", and bulk download/bookmarking
- **Statistics dashboard** — library overview, reading state, recent activity
  and outside-library tracking
- **Local-source support** — `.cbz` archives and folder image collections

## Links

- [Repository](https://github.com/fam007e/mihonQT) ·
  [Issues](https://github.com/fam007e/mihonQT/issues) ·
  [Website](https://fam007e.github.io/mihonQT/) ·
  [FAQ](https://fam007e.github.io/mihonQT/FAQ.html) ·
  [Discord](https://discord.gg/F34duNjTqn)
