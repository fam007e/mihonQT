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

- **Double-page spread reader** with cover offset, wide-page detection and
  RTL/LTR/vertical modes
- **JavaScript extensions** — full HTML5 parsing with CSS selectors, running
  off the UI thread
- **Mihon Android theming** — the same Material 3 palettes, plus AMOLED
- **Library management** — categories, unread/download badges, filters,
  download queue, reading history
- **Trackers-ready** architecture and local-source (CBZ/folder) support

## Links

- [Repository](https://github.com/fam007e/mihonQT) ·
  [Issues](https://github.com/fam007e/mihonQT/issues) ·
  [Website](https://fam007e.github.io/mihonQT/) ·
  [FAQ](https://fam007e.github.io/mihonQT/FAQ.html) ·
  [Discord](https://discord.gg/F34duNjTqn)
