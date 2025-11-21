# MihonQT

MihonQT is a desktop manga reader built with C++ and the Qt framework, inspired by the popular Android application [Mihon](https://mihon.app/). This project is in its early stages, with many features yet to be implemented. Contributions are welcome!

## 🚧 Project Status: Pre-Alpha

This project is currently a work-in-progress. The foundational architecture is in place, but many user-facing features are missing.

## ✨ Features (Planned & Implemented)

-   [x] Basic UI structure with a main window and views for Library, Sources, and Settings.
-   [x] SQLite database for managing manga and chapter information.
-   [x] Polymorphic source architecture for fetching manga from different origins.
-   [x] Support for local manga (`.cbz` format) via `LocalSource`.
-   [x] Extensible Javascript source support for online sources.
-   [ ] Fully implemented reader view.
-   [ ] Comprehensive error handling and user feedback.
-   [ ] Advanced library management (filtering, sorting, categories).
-   [ ] Settings persistence and configuration.

## 🛠️ Building from Source

### Prerequisites

-   C++ Compiler (g++, Clang, MSVC)
-   [CMake](https://cmake.org/) (version 3.16 or higher)
-   [Qt 6](https://www.qt.io/download-qt-installer) (Core, Gui, Widgets, Sql, Concurrent, Quick, Qml)

On Debian-based systems (like Ubuntu), you can install the dependencies with:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake qt6-base-dev
```

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/mihonQT.git
    cd mihonQT
    ```

2.  **Configure the project using CMake:**
    ```bash
    cmake -B build -S .
    ```

3.  **Build the project:**
    ```bash
    cmake --build build
    ```

4.  **Run the application:**
    The executable will be located in the `build` directory.
    ```bash
    ./build/MihonQT
    ```

## 🤝 Contributing

This is a community-driven project, and we welcome contributions of all kinds. Please see the (future) `CONTRIBUTING.md` file for detailed guidelines on how to get involved.

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
