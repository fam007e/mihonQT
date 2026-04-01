# Frequently Asked Questions (FAQ)

## General

### What is MihonQT?
MihonQT is a desktop manga reader inspired by the Mihon (formerly Tachiyomi) Android app. It allows you to read local manga files and install extensions to read from online sources.

### Which platforms are supported?
MihonQT is built with Qt 6 and is designed to run on Linux, Windows, and macOS.

## Extensions

### How do I install extensions?
Go to **More > Extensions**. You can add a repository URL (index.min.json) and browse available extensions to install them.

### Are extensions safe?
Extensions in MihonQT run in a sandbox. To further protect users, we've implemented an **Extension Trust System**:
- **Untrusted (Default):** Extensions are restricted to their source's domain and cannot perform cross-site requests.
- **Trusted:** Users can explicitly "Trust" an extension in the Manager to remove these restrictions.
- **Enforced HTTPS:** You can block all non-secure traffic from extensions in settings.

### What is the user interface inspired by?
MihonQT features a modern, Material 3-inspired design that closely matches the Tachiyomi/Mihon Android app experience, providing a familiar and premium feel for mobile users.

## Reading

### What file formats are supported for local manga?
Currently, MihonQT supports `.cbz` archives and local directories containing images (JPG, PNG, WebP).

### How do I navigate in the reader?
- **Scroll:** Vertically in Webtoon mode, or click/tap the sides in Paged mode.
- **Back:** Right-click anywhere in secondary views, or use the **Back** (み) icon in the top-left sidebar.
- **Keyboard:** Use **Left/Right Arrows** to flip pages, and **Up/Down Arrows** for vertical scrolling.

## Troubleshooting

### The app hangs when I cancel downloads.
This was a known deadlock issue that has been fixed. Please ensure you are running the latest version.

### My extensions are not loading.
Check the **Data Directory** in settings to ensure the extensions are located in the correct folder. You can also try restarting the application.
