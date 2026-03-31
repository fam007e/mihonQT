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
Extensions are JavaScript files that run in a sandbox. However, they are given network and HTML parsing permissions. You should only install extensions from repositories you trust.

## Reading

### What file formats are supported for local manga?
Currently, MihonQT supports `.cbz` archives and local directories containing images (JPG, PNG, WebP).

### How do I navigate in the reader?
- **Scroll:** Use your mouse wheel or touchpad.
- **Back:** Right-click anywhere in the secondary views to return to the main dashboard.
- **Keyboard:** (Coming soon) Arrow keys for page navigation.

## Troubleshooting

### The app hangs when I cancel downloads.
This was a known deadlock issue that has been fixed. Please ensure you are running the latest version.

### My extensions are not loading.
Check the **Data Directory** in settings to ensure the extensions are located in the correct folder. You can also try restarting the application.
