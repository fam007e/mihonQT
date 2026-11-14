# JavaScript Extensions

MihonQT loads manga sources as plain JavaScript files from
`~/.local/share/MihonQT/extensions`. Each file registers a `source` object
(see `extensions/WeebCentral.js` for a full working example) and mirrors the
shape of a Kotlin extension from
[extensions-source](https://github.com/keiyoushi/extensions-source), using
`Html.parse(...).select(...)` instead of Jsoup.

## Source object

```js
var source = {
    id: 1234567,          // unique numeric id
    name: "WeebCentral",
    baseUrl: "https://weebcentral.com",
    supportsLatest: true,

    getPopularManga: function () { ... },   // -> array of manga objects
    getLatestUpdates: function () { ... },
    getSearchManga: function (query) { ... },
    getMangaDetails: function (manga) { ... },
    getChapterList: function (manga) { ... },
    getPageList: function (chapter) { ... },
};
```

Manga objects use the same fields as SManga: `title`, `url`,
`thumbnailUrl`, `author`, `artist`, `description`, `genre` (comma
separated), `status` (0 unknown, 1 ongoing, 2 completed, 3 licensed,
4 publishing finished, 5 cancelled, 6 on hiatus).

Chapters use `url`, `name`, `scanlator`, `chapterNumber`, `dateUpload`
(epoch milliseconds).

## Available globals

### `Network`

| Method | Returns |
|---|---|
| `Network.get(url)` | HTTP body as a string (synchronous, runs off the UI thread) |
| `Network.post(url, data)` | HTTP body as a string |

A browser-like User-Agent is sent and Cloudflare cookies harvested through
the built-in WebView dialog are applied automatically.

### `Html`

| Method | Returns |
|---|---|
| `Html.parse(html)` | Document object |
| `doc.select(css)` | Array of elements (full CSS selector engine: combinators, attributes, `:contains`) |
| `doc.first(css)` | First matching element or null |
| `element.text()` | Clean text (entities decoded, script/style stripped) |
| `element.attr(name)` | Attribute value |
| `element.tagName()` | Lowercased tag name |
| `element.select(css)` / `element.first(css)` | Scoped queries |

### `Console`

| Method | Purpose |
|---|---|
| `Console.log(msg)` | Print to the application log |

## Notes

- HTML parsing is HTML5-correct (Gumbo), so `<picture>`, implicit closes and
  malformed markup behave like a browser.
- Element objects stay alive as long as JavaScript holds them; results of
  `select` are parented to their document.
- Extension files must end with the `source` object registered — see
  `extensions/WeebCentral.js` as the reference implementation.
