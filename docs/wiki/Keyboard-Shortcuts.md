# Keyboard Shortcuts

Focus starts on the **navigation rail** (left side). The app follows a
hub-and-spoke model: the rail is the hub, `Right` enters content, `Esc`
returns.

## Global

| Key | Action |
|---|---|
| `Up` / `Down` | Move between rail sections (Library, Updates, History, Browse, More) |
| `Enter` / `Space` | Open the highlighted rail section |
| `Right` | Enter the current section's content |
| `Left` | Back |
| `Esc` | Back (closes overlays first) |
| `Tab` / `Shift+Tab` | Move between widgets inside a view |
| `Ctrl+Q` | Quit MihonQT |

## Library

| Key | Action |
|---|---|
| Arrows | Move across the manga grid |
| `Enter` | Open the focused manga |
| `s` | Toggle the search bar |

## Manga details

| Key / Mouse | Action |
|---|---|
| `Up` / `Down` | Walk the chapter list (auto-scrolls) |
| `Enter` | Read the focused chapter |
| `f` via filter button | Filter All / Unread / Downloaded / Bookmarked chapters |
| `Shift + Click` | Range select chapters between anchor and clicked row |
| `Right-Click` | Open context menu (Mark Read/Unread, Mark Previous as Read, Download, Bookmark) |
| `Esc` | Clear chapter selection mode / close tracking overlay / back |

## Reader

The reading direction decides which arrow key moves **forward** in the page
count: `Right` in left-to-right, `Left` in right-to-left. This also holds
inside Double Page Spread mode (its layout is always manga-ordered: the
current page on the right, the next page on the left).

| Key | Action |
|---|---|
| `Left` / `Right` (or `A` / `D`) | Backward / forward in the page count (forward key follows the reading direction) |
| `Up` / `Down` | Page step |
| `Space` | Next page |
| `W` | Cycle reading modes: Webtoon → Right-to-Left → Left-to-Right; from Double Page it exits to Webtoon |
| `Shift+D` / `Ctrl+D` | Toggle Double Page Spread (manga-ordered, works in every paged mode; remembered per chapter) |
| `s` | Reader settings + overlay bars |
| `Esc` | Close overlay, then leave the reader |
| Overlay slider | Jump to any page (direction matches RTL modes) |

Tap zones (single-page and double-page modes) follow the same direction rule
as the arrow keys: the forward side of the screen advances the page count,
the middle toggles the settings overlay.

## Webtoon mode

| Key | Action |
|---|---|
| `Space` / `Down` | Scroll to next page |
| Tap zones | Thirds of the screen navigate; middle toggles the overlay |

Double Page Spread does not apply to Webtoon — it is a long strip, so
`Shift+D` is a no-op there and `W` is the way in (and out).
