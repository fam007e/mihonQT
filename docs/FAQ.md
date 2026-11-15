# Frequently Asked Questions (FAQ)

## General

### What is MihonQT?
MihonQT is a desktop manga reader inspired by the Mihon (formerly Tachiyomi) Android app. It allows you to read local manga files and install extensions to read from online sources.

### Which platforms are supported?
MihonQT is built with Qt 6 and is designed to run on Linux, Windows, and macOS.

## Extensions

### How do I install extensions?
Go to **More > Extensions**. The default repository is already configured — it
serves JavaScript sources from the
[`repo-qt` branch of fam007e/extensions-source](https://github.com/fam007e/extensions-source/tree/repo-qt)
(WeebCentral, Toonily, ManhuaUS, HiperDex, Webtoons.com, MangaDex), and more
sources are added over time. You can also add another repository URL (any URL
serving an `index.min.json`) and browse available extensions to install them.

### Are extensions safe?
Extensions in MihonQT run in a sandbox. To further protect users, we've implemented an **Extension Trust System**:
- **Untrusted (Default):** Extensions are restricted to their source's domain and cannot perform cross-site requests.
- **Trusted:** Users can explicitly "Trust" an extension in the Manager to remove these restrictions.
- **Enforced HTTPS:** You can block all non-secure traffic from extensions in settings.

Note: sources that load images or APIs from a different host than their site
(e.g. **MangaDex**, **Webtoons.com**, **HiperDex**) must be marked **Trusted**
before they can fetch anything.

### What is the user interface inspired by?
MihonQT features a modern, Material 3-inspired design that closely matches the Tachiyomi/Mihon Android app experience, providing a familiar and premium feel for mobile users.

## Tracking Services

### Which tracking services are supported?
MihonQT currently supports:
- **AniList** (OAuth2 token authorization)
- **MyAnimeList (MAL)** (OAuth2 PKCE authorization with localhost redirect)
- **MangaUpdates (Baka-Updates)** (API token / user login)

### How do I log in to MyAnimeList (MAL)?
1. Go to **More > Settings > Tracking**.
2. Click **Log in** next to MyAnimeList.
3. If setting up your own MAL Client ID in MyAnimeList Developer settings, ensure the **App Redirect URL** is configured as `http://localhost`.
4. Authorize MihonQT in your browser and complete the sign-in.

### How does tracking progress synchronize?
- **Automatic Chapter Sync:** When you finish reading a chapter in the reader or mark chapters as read in the manga details view, MihonQT automatically sends the updated chapter number to all bound trackers.
- **Pre-filling Local Progress:** If a tracker entry currently has `0` chapters read, opening the tracking dialog automatically pre-fills your highest read chapter number from your local reading history.
- **Score / Rating:** You can set a score from `0.0` to `10.0` (in `0.5` increments) in the tracking dialog and click **Update Progress** to submit your rating directly to the tracking platform.
- **Auto-Search:** Opening the tracking dialog or switching tabs automatically searches the tracker using the current manga title.

## Chapter Management

### How do I select multiple chapters?
1. Click the **Select** button in the chapter list header to enter selection mode.
2. Click a chapter to select it.
3. Hold <kbd>Shift</kbd> and click another chapter — all chapters between the two will be selected immediately.
4. Use the action bar at the top to mark them as read/unread, download, or bookmark in batch.
5. Press <kbd>Esc</kbd> or click the back arrow to exit selection mode.

### How does "Mark previous chapters as read" work?
Right-click any chapter in the list and choose **Mark previous chapters as read**.
- MihonQT will mark all chapters *before* the chosen chapter as read in reading order (ascending).
- The selected chapter itself remains **unread**, so you can start reading right from it without having to uncheck it.
- Your connected trackers will automatically update to reflect the last read chapter.

## Reading

### What file formats are supported for local manga?
Currently, MihonQT supports `.cbz` archives and local directories containing images (JPG, PNG, WebP).

### How do I navigate in the reader?
- **Scroll:** Vertically in Webtoon mode, or click/tap the sides in Paged mode.
- **Keyboard:** Left/Right (or `A`/`D`) flips pages (direction follows the reading mode), Space steps forward, `W` or `Shift+D`/`Ctrl+D` toggles Double Page Spread mode, and `s` opens the reader settings overlay.
- **Back:** `Esc` closes overlays first, then leaves the view; `Ctrl+Q` quits the app.
- The full navigation map lives in the **[Keyboard Shortcuts](https://github.com/fam007e/mihonQT/wiki/Keyboard-Shortcuts)** page.

## Troubleshooting

### The app hangs when I cancel downloads.
This was a known deadlock issue that has been fixed. Please ensure you are running the latest version.

### My extensions are not loading.
Check the **Data Directory** in settings to ensure the extensions are located in the correct folder. You can also try restarting the application.
