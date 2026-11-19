# Themes

MihonQT ships the Mihon Android color schemes as Material 3 token sets. All
component colors come from these tokens — there are no per-widget hardcoded
colors left in the main views.

## Available themes

| Theme | Notes |
|---|---|
| **Mihon Default** (default) | The upstream Material 3 scheme; primary `#B0C6FF` on dark |
| Nord Dark / Nord Light | Ported from Mihon's NordColorScheme |
| Catppuccin Mocha | Upstream Catppuccin scheme (light "Frappe" entry maps to Mocha tokens) |
| Tokyo Night | Ported from TokyoNightColorScheme |
| Dracula | Desktop-only extra, not in Mihon upstream |

## Pure Black (AMOLED)

Settings → Appearance → *Pure Black (AMOLED)* replaces window/surface colors
with pure black while keeping the fixed surface containers readable — same
behavior as Mihon on Android.

## Switching

- **Settings → Appearance → Application Theme** (persisted)
- Reader is unaffected: pages always render on black

## Widget Style (native Qt styles)

Beyond the built-in Material look, **Settings → Appearance → Widget Style**
renders the app with a native Qt style: **Material (app theme)** (default),
**System default** (whatever the platform theme picked at launch), or any
installed style — **Breeze**, **Oxygen**, **Kvantum**, **Fusion**, ...

- Reader pages always render on black; the widget style changes the app
  chrome (menus, buttons, lists, dropdowns).
- While a native style is active, the Material theme and AMOLED toggles are
  paused — they apply again as soon as you switch back to *Material*.
- Translucent Kvantum themes would normally leave combobox dropdowns
  unreadable; MihonQT forces those popups opaque.
- The Statistics dashboard, chapter lists and all other custom-drawn parts
  stay theme-consistent in both modes (colors derive from the active
  palette).

## Token model

Colors live in `src/ui/ThemeManager.cpp` as exact ports of the upstream
`scheme.kt` files and are exposed as a `M3Colors` struct (`primary`,
`secondaryContainer`, `tertiary`, `surfaceContainerHigh`, ...). Widgets must
read tokens from `ThemeManager::instance().colors()` — hardcoded hex in
components defeats the theming system.
