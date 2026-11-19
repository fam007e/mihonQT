# Frequently Asked Questions (FAQ)

## General

### What is MihonQT?
MihonQT is a desktop manga reader inspired by the Mihon (formerly Tachiyomi) Android app. It allows you to read local manga files and install extensions to read from online sources.

MihonQT is an **independent community project** — it is not affiliated with, endorsed by, or connected to the Mihon or Tachiyomi development teams.

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

### How can I support the project?
There is no donation setup for MihonQT. The **Donate** button in **More** opens
the [Support page](https://fam007e.github.io/mihonQT/DONATE.html), which
explains the situation and points at the places where support helps most —
chiefly **[Mihon](https://github.com/mihonapp/mihon)** itself, the app MihonQT
is a port of. Filing good bug reports, helping users, and improving the docs
all count too.

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
- **Keyboard:** `Right`/`Left` (or `D`/`A`) move forward/backward in the page count — which arrow key is *forward* follows the reading direction (`Right` in Left-to-Right, `Left` in Right-to-Left), also inside Double Page Spread. `Space` steps forward, `W` cycles reading modes (Webtoon → Right-to-Left → Left-to-Right) and leaves Double Page to Webtoon, `Shift+D`/`Ctrl+D` toggles Double Page Spread, and `s` opens the reader settings overlay.
- **Back:** `Esc` closes overlays first, then leaves the view; `Ctrl+Q` quits the app.
- The full navigation map lives in the **[Keyboard Shortcuts](https://github.com/fam007e/mihonQT/wiki/Keyboard-Shortcuts)** page.

### How does Double Page Spread work?
Toggle it with `Shift+D`/`Ctrl+D` (or **Double Page** in the reader settings). The
spread is always **manga-ordered** — the current page sits on the right and the
next page on the left, matching how manga artwork is drawn — in every reading
mode.
- **Remembered per chapter:** close the app mid-chapter and reopening that
  chapter resumes in double-page; starting a *different* chapter begins in your
  base reading mode again until you toggle it.
- **Re-aligning:** the spread pairs the page you toggled on with the next one —
  toggle while standing on a spread's left half and the artwork lines up.
- **Webtoon** is a long strip, so double-page doesn't apply there (`Shift+D` is
  a no-op) and `W` is the way in.
- **Options:** *First page is cover* keeps page 1 single; *Don't pair wide
  pages* shows pre-merged splash images alone instead of pairing them.

### Why does "Mark as unread" send me back to page 1?
Marking a chapter unread resets its saved page, so it reopens from the first
page — the point of unreading. If you only want to keep reading where you
stopped, don't toggle its read state.

### Why did the statistics numbers change?
Statistics are now a **library dashboard** and count your **library** only:
content that was synced while browsing a manga you never added (and its
chapters) is reported separately under **Outside library**. You also get new
sections — Reading state (started/completed) and Activity (chapters read in
the last 7 and 30 days, last-read date).

## Troubleshooting

### The app hangs when I cancel downloads.
This was a known deadlock issue that has been fixed. Please ensure you are running the latest version.

### My extensions are not loading.
Check the **Data Directory** in settings to ensure the extensions are located in the correct folder. You can also try restarting the application.
