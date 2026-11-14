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

## Token model

Colors live in `src/ui/ThemeManager.cpp` as exact ports of the upstream
`scheme.kt` files and are exposed as a `M3Colors` struct (`primary`,
`secondaryContainer`, `tertiary`, `surfaceContainerHigh`, ...). Widgets must
read tokens from `ThemeManager::instance().colors()` — hardcoded hex in
components defeats the theming system.
