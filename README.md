<p align="center">
  <img src="docs/logo.png" width="160" height="160" alt="MihonQT Logo">
</p>

<h1 align="center">MihonQT</h1>
<p align="center">
  <a href="https://github.com/fam007e/mihonQT/actions/workflows/build.yml">
    <img src="https://github.com/fam007e/mihonQT/actions/workflows/build.yml/badge.svg" alt="Build Status">
  </a>
  <a href="https://github.com/fam007e/mihonQT/actions/workflows/security.yml">
    <img src="https://github.com/fam007e/mihonQT/actions/workflows/security.yml/badge.svg" alt="Security Scan">
  </a>
  <a href="https://github.com/fam007e/mihonQT/actions/workflows/release.yml">
    <img src="https://github.com/fam007e/mihonQT/actions/workflows/release.yml/badge.svg" alt="Release Status">
  </a>
  <a href="https://github.com/fam007e/mihonQT/actions/workflows/static.yml">
    <img src="https://github.com/fam007e/mihonQT/actions/workflows/static.yml/badge.svg" alt="Documentation Status">
  </a>
</p>

MihonQT is a modern, extensible, and secure desktop manga reader built with C++ and Qt 6, inspired by the popular [Mihon](https://mihon.app/) (formerly Tachiyomi) Android application.

## ✨ Features

- **📚 Library Management:** Organize your manga with categories and track your reading progress.
- **🌐 Extension System:** Add third-party repositories with a built-in **Extension Trust System** to sandbox online sources.
- **🚀 Fast Reader:** Support for local directories and `.cbz` archives with smooth vertical (Webtoon) and paged reading modes.
- **🕶️ Incognito Mode:** Pause history tracking and reading progress with a single toggle.
- **🎨 Material 3 Inspired UI:** A beautiful, responsive desktop interface consistent with Tachiyomi/Mihon Android.
- **📥 Download Manager:** Queue chapters for offline reading with a robust, multi-threaded download manager.
- **📊 Statistics:** Get insights into your reading habits with a dedicated statistics dashboard.
- **🔒 Secure by Design:** Incorporating mitigations from the 2026 Security Audit for extension sandboxing and network security.

## 🚀 Getting Started

### Prerequisites

-   **CMake** (3.16+)
-   **Qt 6** (Core, Gui, Widgets, Network, Sql, Concurrent)
-   **QuaZip** (for `.cbz` support)

### Building

```bash
cmake -B build
cmake --build build
./build/MihonQT
```

## 🔒 Security & Privacy

MihonQT is designed with privacy in mind.
- **No Tracking:** We do not collect any personal data.
- **Incognito Mode:** Easily pause tracking when needed.
- **Local First:** Your database and settings stay on your machine.

For a detailed security analysis, see [docs/SECURITY_AUDIT.md](docs/SECURITY_AUDIT.md).

## 🗺️ Roadmap

See [ROADMAP.md](ROADMAP.md) for our progress and planned features.

## 🤝 Contributing

Contributions are welcome! Please check out our [CONTRIBUTING.md](CONTRIBUTING.md) to get started.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
