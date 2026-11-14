#include "ThemeManager.h"
#include <QStyleFactory>

// ============================================================
// Color schemes — ported 1:1 from Mihon Android
// (presentation/theme/colorscheme/*.kt) so the desktop app uses
// the exact same Material 3 palettes.
// ============================================================

static M3Colors mihonDefaultDark()
{
    // TachiyomiColorScheme.kt — Default (M3, Material Theme Builder)
    M3Colors c;
    c.primary = QRgb(0xB0C6FF); c.onPrimary = QRgb(0x002D6E);
    c.primaryContainer = QRgb(0x00429B); c.onPrimaryContainer = QRgb(0xD9E2FF);
    c.secondary = QRgb(0xB0C6FF); c.onSecondary = QRgb(0x002D6E); // unread badge
    c.secondaryContainer = QRgb(0x00429B); c.onSecondaryContainer = QRgb(0xD9E2FF);
    c.tertiary = QRgb(0x7ADC77); c.onTertiary = QRgb(0x003909); // downloaded badge
    c.tertiaryContainer = QRgb(0x005312); c.onTertiaryContainer = QRgb(0x95F990);
    c.background = QRgb(0x1B1B1F); c.onBackground = QRgb(0xE3E2E6);
    c.surface = QRgb(0x1B1B1F); c.onSurface = QRgb(0xE3E2E6);
    c.surfaceVariant = QRgb(0x211F26); c.onSurfaceVariant = QRgb(0xC5C6D0);
    c.surfaceContainerLowest = QRgb(0x1A181D); c.surfaceContainerLow = QRgb(0x1E1C22);
    c.surfaceContainer = QRgb(0x211F26); c.surfaceContainerHigh = QRgb(0x292730);
    c.surfaceContainerHighest = QRgb(0x302E38);
    c.outline = QRgb(0x8F9099); c.outlineVariant = QRgb(0x44464F);
    c.error = QRgb(0xFFB4AB); c.onError = QRgb(0x690005);
    c.errorContainer = QRgb(0x93000A); c.onErrorContainer = QRgb(0xFFDAD6);
    c.inverseSurface = QRgb(0xE3E2E6); c.inverseOnSurface = QRgb(0x1B1B1F);
    return c;
}

static M3Colors mihonDefaultLight()
{
    M3Colors c;
    c.primary = QRgb(0x0058CA); c.onPrimary = QRgb(0xFFFFFF);
    c.primaryContainer = QRgb(0xD9E2FF); c.onPrimaryContainer = QRgb(0x001945);
    c.secondary = QRgb(0x0058CA); c.onSecondary = QRgb(0xFFFFFF);
    c.secondaryContainer = QRgb(0xD9E2FF); c.onSecondaryContainer = QRgb(0x001945);
    c.tertiary = QRgb(0x006E1B); c.onTertiary = QRgb(0xFFFFFF);
    c.tertiaryContainer = QRgb(0x95F990); c.onTertiaryContainer = QRgb(0x002203);
    c.background = QRgb(0xFEFBFF); c.onBackground = QRgb(0x1B1B1F);
    c.surface = QRgb(0xFEFBFF); c.onSurface = QRgb(0x1B1B1F);
    c.surfaceVariant = QRgb(0xF3EDF7); c.onSurfaceVariant = QRgb(0x44464F);
    c.surfaceContainerLowest = QRgb(0xF5F1F8); c.surfaceContainerLow = QRgb(0xF7F2FA);
    c.surfaceContainer = QRgb(0xF3EDF7); c.surfaceContainerHigh = QRgb(0xFCF7FF);
    c.surfaceContainerHighest = QRgb(0xFCF7FF);
    c.outline = QRgb(0x757780); c.outlineVariant = QRgb(0xC5C6D0);
    c.error = QRgb(0xBA1A1A); c.onError = QRgb(0xFFFFFF);
    c.errorContainer = QRgb(0xFFDAD6); c.onErrorContainer = QRgb(0x410002);
    c.inverseSurface = QRgb(0x303034); c.inverseOnSurface = QRgb(0xF2F0F4);
    return c;
}

static M3Colors nordDark()
{
    // NordColorScheme.kt
    M3Colors c;
    c.primary = QRgb(0x88C0D0); c.onPrimary = QRgb(0x2E3440);
    c.primaryContainer = QRgb(0x88C0D0); c.onPrimaryContainer = QRgb(0x2E3440);
    c.secondary = QRgb(0x81A1C1); c.onSecondary = QRgb(0x2E3440); // unread badge
    c.secondaryContainer = QRgb(0x506275); c.onSecondaryContainer = QRgb(0x88C0D0);
    c.tertiary = QRgb(0x5E81AC); c.onTertiary = QRgb(0x000000); // downloaded badge
    c.tertiaryContainer = QRgb(0x5E81AC); c.onTertiaryContainer = QRgb(0x000000);
    c.background = QRgb(0x2E3440); c.onBackground = QRgb(0xECEFF4);
    c.surface = QRgb(0x2E3440); c.onSurface = QRgb(0xECEFF4);
    c.surfaceVariant = QRgb(0x414C5C); c.onSurfaceVariant = QRgb(0xECEFF4);
    c.surfaceContainerLowest = QRgb(0x373F4D); c.surfaceContainerLow = QRgb(0x3E4756);
    c.surfaceContainer = QRgb(0x414C5C); c.surfaceContainerHigh = QRgb(0x4E5766);
    c.surfaceContainerHighest = QRgb(0x505968);
    c.outline = QRgb(0x6D717B); c.outlineVariant = QRgb(0x90939A);
    c.error = QRgb(0xBF616A); c.onError = QRgb(0x2E3440);
    c.errorContainer = QRgb(0xBF616A); c.onErrorContainer = QRgb(0x000000);
    c.inverseSurface = QRgb(0xD8DEE9); c.inverseOnSurface = QRgb(0x2E3440);
    return c;
}

static M3Colors nordLight()
{
    M3Colors c;
    c.primary = QRgb(0x5E81AC); c.onPrimary = QRgb(0x000000);
    c.primaryContainer = QRgb(0x5E81AC); c.onPrimaryContainer = QRgb(0x000000);
    c.secondary = QRgb(0x81A1C1); c.onSecondary = QRgb(0x2E3440);
    c.secondaryContainer = QRgb(0x91B4D7); c.onSecondaryContainer = QRgb(0x2E3440);
    c.tertiary = QRgb(0x88C0D0); c.onTertiary = QRgb(0x2E3440);
    c.tertiaryContainer = QRgb(0x88C0D0); c.onTertiaryContainer = QRgb(0x2E3440);
    c.background = QRgb(0xECEFF4); c.onBackground = QRgb(0x2E3440);
    c.surface = QRgb(0xE5E9F0); c.onSurface = QRgb(0x2E3440);
    c.surfaceVariant = QRgb(0xDAE0EA); c.onSurfaceVariant = QRgb(0x2E3440);
    c.surfaceContainerLowest = QRgb(0xD1D7E0); c.surfaceContainerLow = QRgb(0xD6DCE6);
    c.surfaceContainer = QRgb(0xDAE0EA); c.surfaceContainerHigh = QRgb(0xE9EDF3);
    c.surfaceContainerHighest = QRgb(0xF2F4F8);
    c.outline = QRgb(0x2E3440); c.outlineVariant = QRgb(0xD8DEE9);
    c.error = QRgb(0xBF616A); c.onError = QRgb(0xECEFF4);
    c.errorContainer = QRgb(0xBF616A); c.onErrorContainer = QRgb(0x000000);
    c.inverseSurface = QRgb(0x3B4252); c.inverseOnSurface = QRgb(0xECEFF4);
    return c;
}

static M3Colors catppuccinMochaDark()
{
    // CatppuccinColorScheme.kt (Mocha)
    M3Colors c;
    c.primary = QRgb(0xCBA6F7); c.onPrimary = QRgb(0x11111B);
    c.primaryContainer = QRgb(0xCBA6F7); c.onPrimaryContainer = QRgb(0x11111B);
    c.secondary = QRgb(0xB4BEFE); c.onSecondary = QRgb(0x11111B);
    c.secondaryContainer = QRgb(0x313244); c.onSecondaryContainer = QRgb(0xCBA6F7);
    c.tertiary = QRgb(0xA6E3A1); c.onTertiary = QRgb(0x11111B);
    c.tertiaryContainer = QRgb(0x1E1E2E); c.onTertiaryContainer = QRgb(0xCDD6F4);
    c.background = QRgb(0x181825); c.onBackground = QRgb(0xCDD6F4);
    c.surface = QRgb(0x181825); c.onSurface = QRgb(0xCDD6F4);
    c.surfaceVariant = QRgb(0x1E1E2E); c.onSurfaceVariant = QRgb(0xCDD6F4);
    c.surfaceContainerLowest = QRgb(0x181825); c.surfaceContainerLow = QRgb(0x1E1E2E);
    c.surfaceContainer = QRgb(0x1E1E2E); c.surfaceContainerHigh = QRgb(0x1E1E2E);
    c.surfaceContainerHighest = QRgb(0x313244);
    c.outline = QRgb(0xCBA6F7); c.outlineVariant = QRgb(0x585B70);
    c.error = QRgb(0xF38BA8); c.onError = QRgb(0x11111B);
    c.errorContainer = QRgb(0xFF0558); c.onErrorContainer = QRgb(0xEF9FB4);
    c.inverseSurface = QRgb(0xEFF1F5); c.inverseOnSurface = QRgb(0x4C4F69);
    return c;
}

static M3Colors catppuccinLight()
{
    // Catppuccin Latte (light variant of the same scheme)
    M3Colors c;
    c.primary = QRgb(0x8839EF); c.onPrimary = QRgb(0xDCE0E8);
    c.primaryContainer = QRgb(0x8839EF); c.onPrimaryContainer = QRgb(0xDCE0E8);
    c.secondary = QRgb(0x7287FD); c.onSecondary = QRgb(0xDCE0E8);
    c.secondaryContainer = QRgb(0xCDD0DA); c.onSecondaryContainer = QRgb(0x8839EF);
    c.tertiary = QRgb(0x40A02B); c.onTertiary = QRgb(0xDCE0E8);
    c.tertiaryContainer = QRgb(0xEFF1F5); c.onTertiaryContainer = QRgb(0x4C4F69);
    c.background = QRgb(0xE6E9EF); c.onBackground = QRgb(0x4C4F69);
    c.surface = QRgb(0xE6E9EF); c.onSurface = QRgb(0x4C4F69);
    c.surfaceVariant = QRgb(0xEFF1F5); c.onSurfaceVariant = QRgb(0x4C4F69);
    c.surfaceContainerLowest = QRgb(0xE6E9EF); c.surfaceContainerLow = QRgb(0xEFF1F5);
    c.surfaceContainer = QRgb(0xEFF1F5); c.surfaceContainerHigh = QRgb(0xEFF1F5);
    c.surfaceContainerHighest = QRgb(0xCDD0DA);
    c.outline = QRgb(0x8839EF); c.outlineVariant = QRgb(0xACB0BE);
    c.error = QRgb(0xD20F39); c.onError = QRgb(0xDCE0E8);
    c.errorContainer = QRgb(0x68001C); c.onErrorContainer = QRgb(0xD61C41);
    c.inverseSurface = QRgb(0x1E1E2E); c.inverseOnSurface = QRgb(0xCDD6F4);
    return c;
}

static M3Colors tokyoNightDark()
{
    // TokyoNightColorScheme.kt
    M3Colors c;
    c.primary = QRgb(0x82AAFF); c.onPrimary = QRgb(0x1B1D2B);
    c.primaryContainer = QRgb(0x444A73); c.onPrimaryContainer = QRgb(0x9AB8FF);
    c.secondary = QRgb(0x86E1FC); c.onSecondary = QRgb(0x1B1D2B);
    c.secondaryContainer = QRgb(0x444A73); c.onSecondaryContainer = QRgb(0xB2EBFF);
    c.tertiary = QRgb(0xC099FF); c.onTertiary = QRgb(0x1B1D2B);
    c.tertiaryContainer = QRgb(0x444A73); c.onTertiaryContainer = QRgb(0xCAABFF);
    c.background = QRgb(0x222436); c.onBackground = QRgb(0xC8D3F5);
    c.surface = QRgb(0x222436); c.onSurface = QRgb(0xC8D3F5);
    c.surfaceVariant = QRgb(0x444A73); c.onSurfaceVariant = QRgb(0x828BB8);
    c.surfaceContainerLowest = QRgb(0x1E202F); c.surfaceContainerLow = QRgb(0x292C44);
    c.surfaceContainer = QRgb(0x2D304B); c.surfaceContainerHigh = QRgb(0x363B5D);
    c.surfaceContainerHighest = QRgb(0x3F466F);
    c.outline = QRgb(0x828BB8); c.outlineVariant = QRgb(0x444A73);
    c.error = QRgb(0xFF757F); c.onError = QRgb(0x1B1D2B);
    c.errorContainer = QRgb(0xC53B53); c.onErrorContainer = QRgb(0xFF8D94);
    c.inverseSurface = QRgb(0xC8D3F5); c.inverseOnSurface = QRgb(0x222436);
    return c;
}

static M3Colors tokyoNightLight()
{
    M3Colors c;
    c.primary = QRgb(0x2E7DE9); c.onPrimary = QRgb(0xFFFFFF);
    c.primaryContainer = QRgb(0x92A6D5); c.onPrimaryContainer = QRgb(0x15386A);
    c.secondary = QRgb(0x007197); c.onSecondary = QRgb(0xFFFFFF);
    c.secondaryContainer = QRgb(0xA9C6D3); c.onSecondaryContainer = QRgb(0x006A83);
    c.tertiary = QRgb(0x9854F1); c.onTertiary = QRgb(0xFFFFFF);
    c.tertiaryContainer = QRgb(0xCFBFEA); c.onTertiaryContainer = QRgb(0x7847BD);
    c.background = QRgb(0xE1E2E7); c.onBackground = QRgb(0x3760BF);
    c.surface = QRgb(0xE1E2E7); c.onSurface = QRgb(0x3760BF);
    c.surfaceVariant = QRgb(0xC4C8DA); c.onSurfaceVariant = QRgb(0x6172B0);
    c.surfaceContainerLowest = QRgb(0xE4E5E9); c.surfaceContainerLow = QRgb(0xD0D5E3);
    c.surfaceContainer = QRgb(0xC1C9DF); c.surfaceContainerHigh = QRgb(0xC4C8DA);
    c.surfaceContainerHighest = QRgb(0xA1A6C5);
    c.outline = QRgb(0x68709A); c.outlineVariant = QRgb(0xA8AECB);
    c.error = QRgb(0xF52A65); c.onError = QRgb(0xFFFFFF);
    c.errorContainer = QRgb(0xE6B4C7); c.onErrorContainer = QRgb(0x6E132D);
    c.inverseSurface = QRgb(0x3760BF); c.inverseOnSurface = QRgb(0xE1E2E7);
    return c;
}

static M3Colors draculaDark()
{
    // Not in Mihon upstream; kept as an extra desktop theme, tokenized.
    M3Colors c;
    c.primary = QRgb(0xBD93F9); c.onPrimary = QRgb(0x282A36);
    c.primaryContainer = QRgb(0xBD93F9); c.onPrimaryContainer = QRgb(0x282A36);
    c.secondary = QRgb(0x8BE9FD); c.onSecondary = QRgb(0x282A36);
    c.secondaryContainer = QRgb(0x44475A); c.onSecondaryContainer = QRgb(0x8BE9FD);
    c.tertiary = QRgb(0x50FA7B); c.onTertiary = QRgb(0x282A36);
    c.tertiaryContainer = QRgb(0x44475A); c.onTertiaryContainer = QRgb(0x50FA7B);
    c.background = QRgb(0x282A36); c.onBackground = QRgb(0xF8F8F2);
    c.surface = QRgb(0x282A36); c.onSurface = QRgb(0xF8F8F2);
    c.surfaceVariant = QRgb(0x44475A); c.onSurfaceVariant = QRgb(0xCBCBD6);
    c.surfaceContainerLowest = QRgb(0x21222C); c.surfaceContainerLow = QRgb(0x343746);
    c.surfaceContainer = QRgb(0x3B3E51); c.surfaceContainerHigh = QRgb(0x44475A);
    c.surfaceContainerHighest = QRgb(0x4B4E63);
    c.outline = QRgb(0x8B8DA0); c.outlineVariant = QRgb(0x585B70);
    c.error = QRgb(0xFF5555); c.onError = QRgb(0x282A36);
    c.errorContainer = QRgb(0x8B2B2B); c.onErrorContainer = QRgb(0xFFB4B4);
    c.inverseSurface = QRgb(0xF8F8F2); c.inverseOnSurface = QRgb(0x282A36);
    return c;
}

// ============================================================

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    // Seed tokens immediately: views are constructed before MainWindow calls
    // applyTheme(), and default-constructed QColors render as black, which
    // produced black-on-black selected items in History/Settings.
    m_colors = mihonDefaultDark();
    m_isDark = true;
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

bool ThemeManager::isAmoledMode() const
{
    return m_isAmoled;
}

void ThemeManager::setAmoledMode(bool enabled)
{
    if (m_isAmoled == enabled) return;
    m_isAmoled = enabled;
    applyTheme(m_currentTheme); // Re-apply current theme to update colors
}

QString ThemeManager::rgba(const QColor& c, float alpha)
{
    QColor a = c;
    a.setAlphaF(qBound(0.0f, alpha, 1.0f));
    return a.name(QColor::HexArgb);
}

void ThemeManager::applyTheme(Theme theme)
{
    m_currentTheme = theme;
    qApp->setStyle(QStyleFactory::create("Fusion"));

    switch (theme) {
    case MihonDefault:    setTokens(mihonDefaultDark(), mihonDefaultLight(), true); break;
    case NordDark:        setTokens(nordDark(), nordLight(), true); break;
    case NordLight:       setTokens(nordDark(), nordLight(), false); break;
    case CatppuccinMocha: setTokens(catppuccinMochaDark(), catppuccinLight(), true); break;
    case CatppuccinFrappe:setTokens(catppuccinMochaDark(), catppuccinLight(), true); break; // fallback to Mocha tokens
    case TokyoNight:      setTokens(tokyoNightDark(), tokyoNightLight(), true); break;
    case Dracula:         setTokens(draculaDark(), nordLight(), true); break;
    }

    if (m_isAmoled) applyAmoledOverride();

    applyPalette();
    applyGlobalQss();
    emit themeChanged(theme);
}

void ThemeManager::setTokens(const M3Colors& dark, const M3Colors& light, bool darkMode)
{
    m_colors = darkMode ? dark : light;
    m_isDark = darkMode;
}

void ThemeManager::applyAmoledOverride()
{
    // Mirrors BaseColorScheme.kt AMOLED handling: pure black surfaces,
    // but keep the fixed surface containers so scrolling content stays readable.
    static const QColor sc = QRgb(0x0C0C0C), scHigh = QRgb(0x131313), scHighest = QRgb(0x1B1B1B);
    m_colors.background = QColor(Qt::black);
    m_colors.onBackground = QColor(Qt::white);
    m_colors.surface = QColor(Qt::black);
    m_colors.onSurface = QColor(Qt::white);
    m_colors.surfaceVariant = sc;
    m_colors.surfaceContainerLowest = sc;
    m_colors.surfaceContainerLow = sc;
    m_colors.surfaceContainer = sc;
    m_colors.surfaceContainerHigh = scHigh;
    m_colors.surfaceContainerHighest = scHighest;
}

void ThemeManager::applyPalette()
{
    const M3Colors& c = m_colors;
    QPalette p;
    p.setColor(QPalette::Window, c.background);
    p.setColor(QPalette::WindowText, c.onBackground);
    p.setColor(QPalette::Base, c.surfaceContainerLow);
    p.setColor(QPalette::AlternateBase, c.surfaceContainer);
    p.setColor(QPalette::ToolTipBase, c.inverseSurface);
    p.setColor(QPalette::ToolTipText, c.inverseOnSurface);
    p.setColor(QPalette::Text, c.onSurface);
    p.setColor(QPalette::Button, c.surfaceContainer);
    p.setColor(QPalette::ButtonText, c.onSurface);
    p.setColor(QPalette::BrightText, c.primary);
    p.setColor(QPalette::Link, c.primary);
    p.setColor(QPalette::Highlight, c.secondaryContainer);
    p.setColor(QPalette::HighlightedText, c.onSecondaryContainer);
    p.setColor(QPalette::PlaceholderText, c.onSurfaceVariant);
    p.setColor(QPalette::Disabled, QPalette::Text, rgba(c.onSurface, 0.38f));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, rgba(c.onSurface, 0.38f));
    qApp->setPalette(p);
}

void ThemeManager::applyGlobalQss()
{
    const M3Colors& c = m_colors;
    const bool dark = m_isDark;

    QString qss = QStringLiteral(
        // --- Base ---
        "QWidget { background-color: %BG%; color: %ONBG%; }"
        "QMainWindow, QDialog { background-color: %BG%; }"
        "QLabel { background: transparent; }"

        // --- Text input: Mihon search-bar look (rounded, no border) ---
        "QLineEdit { background-color: %SURF_HIGH%; border: 2px solid transparent;"
        "  border-radius: 14px; padding: 8px 14px; color: %ONSURF%; selection-background-color: %PRIM%; }"
        "QLineEdit:focus { border-color: %PRIM%; background-color: %SURF_LOW%; }"
        "QLineEdit::placeholder { color: %ONSURFVAR%; }"

        // --- Buttons: M3 filled (default) / tonal ---
        "QPushButton { background-color: %PRIM%; color: %ONPRIM%; border: none;"
        "  border-radius: 22px; padding: 9px 22px; font-weight: 500; }"
        "QPushButton:hover { background-color: %HOVER_OVER_PRIM%; }"
        "QPushButton:pressed { background-color: %PRIM_CONTAINER%; }"
        "QPushButton:disabled { background-color: rgba(127,127,127,0.24); color: rgba(127,127,127,0.38); }"

        // --- Flat icon buttons used in toolbars ---
        "QPushButton[flatIcon=\"true\"] { background: transparent; border-radius: 20px; padding: 0; }"
        "QPushButton[flatIcon=\"true\"]:hover { background-color: rgba(127,127,127,0.16); }"

        // --- Menus ---
        "QMenu { background-color: %SURF_CONTAINER%; border: 1px solid %OUTLINE_VAR%; border-radius: 12px; padding: 6px; }"
        "QMenu::item { padding: 8px 26px 8px 14px; border-radius: 8px; color: %ONSURF%; }"
        "QMenu::item:selected { background-color: rgba(127,127,127,0.16); }"
        "QMenu::separator { height: 1px; background-color: %OUTLINE_VAR%; margin: 4px 8px; }"

        // --- Lists ---
        "QListWidget { background-color: transparent; border: none; outline: none; }"
        "QListWidget::item { padding: 10px 14px; border-radius: 10px; color: %ONSURF%; }"
        "QListWidget::item:hover { background-color: rgba(127,127,127,0.12); }"
        "QListWidget::item:selected { background-color: %SEC_CONTAINER%; color: %ONSEC_CONTAINER%; }"

        // --- Scrollbars: slim Mihon-style ---
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: rgba(127,127,127,0.35); border-radius: 4px; min-height: 40px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(127,127,127,0.55); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        "QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }"
        "QScrollBar::handle:horizontal { background: rgba(127,127,127,0.35); border-radius: 4px; min-width: 40px; }"
        "QScrollBar::handle:horizontal:hover { background: rgba(127,127,127,0.55); }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }"

        // --- Combo boxes ---
        "QComboBox { background-color: %SURF_HIGH%; border: none; border-radius: 14px;"
        "  padding: 7px 14px; color: %ONSURF%; }"
        "QComboBox:hover { background-color: %SURF_HIGHEST%; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox QAbstractItemView { background-color: %SURF_CONTAINER%; border: 1px solid %OUTLINE_VAR%;"
        "  border-radius: 10px; selection-background-color: rgba(127,127,127,0.2); selection-color: %ONSURF%; outline: none; }"

        // --- Tabs (Mihon library tab row): primary underline indicator ---
        "QTabBar { background: transparent; }"
        "QTabBar::tab { background: transparent; color: %ONSURFVAR%; padding: 10px 18px;"
        "  border: none; border-bottom: 3px solid transparent; font-weight: 500; }"
        "QTabBar::tab:selected { color: %ONBG%; border-bottom-color: %PRIM%; }"
        "QTabBar::tab:hover { color: %ONBG%; }"

        // --- Checkboxes ---
        "QCheckBox { spacing: 10px; color: %ONSURF%; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border: 2px solid %OUTLINE%;"
        "  border-radius: 5px; background: transparent; }"
        "QCheckBox::indicator:checked { background-color: %PRIM%; border-color: %PRIM%;"
        "  image: url(:/icons/check.svg); }"

        // --- Sliders ---
        "QSlider::groove:horizontal { height: 6px; border-radius: 3px; background: %SURF_HIGHEST%; }"
        "QSlider::sub-page:horizontal { background: %PRIM%; border-radius: 3px; }"
        "QSlider::handle:horizontal { width: 18px; height: 18px; margin: -6px 0;"
        "  border-radius: 9px; background: %PRIM%; }"
        "QSlider::groove:vertical { width: 6px; border-radius: 3px; background: %SURF_HIGHEST%; }"
        "QSlider::handle:vertical { height: 18px; width: 18px; margin: 0 -6px;"
        "  border-radius: 9px; background: %PRIM%; }"

        // --- Progress ---
        "QProgressBar { border: none; background: rgba(127,127,127,0.18); border-radius: 2px; }"
        "QProgressBar::chunk { background-color: %PRIM%; border-radius: 2px; }"

        // --- Tooltips / groups / split ---
        "QToolTip { background-color: %INV_SURF%; color: %ONINV_SURF%; border: none; padding: 6px 8px; border-radius: 6px; }"
        "QGroupBox { border: 1px solid %OUTLINE_VAR%; border-radius: 12px; margin-top: 14px;"
        "  padding-top: 10px; color: %ONSURF%; font-weight: 600; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; top: 2px; color: %ONSURFVAR%; }"
        "QSplitter::handle { background: transparent; }"
        "QStatusBar { background: %SURF_CONTAINER%; color: %ONSURFVAR%; }"
    );

    // Token substitution
    struct { const char* key; QString value; } subs[] = {
        { "%BG%",             c.background.name() },
        { "%ONBG%",           c.onBackground.name() },
        { "%SURF%",           c.surface.name() },
        { "%ONSURF%",         c.onSurface.name() },
        { "%SURF_LOW%",       c.surfaceContainerLow.name() },
        { "%SURF_CONTAINER%", c.surfaceContainer.name() },
        { "%SURF_HIGH%",      c.surfaceContainerHigh.name() },
        { "%SURF_HIGHEST%",   c.surfaceContainerHighest.name() },
        { "%ONSURFVAR%",      c.onSurfaceVariant.name() },
        { "%PRIM%",           c.primary.name() },
        { "%ONPRIM%",         c.onPrimary.name() },
        { "%PRIM_CONTAINER%", c.primaryContainer.name() },
        { "%HOVER_OVER_PRIM%", ThemeManager::rgba(c.primary, dark ? 0.85f : 0.9f) },
        { "%SEC_CONTAINER%",  c.secondaryContainer.name() },
        { "%ONSEC_CONTAINER%",c.onSecondaryContainer.name() },
        { "%OUTLINE%",        c.outline.name() },
        { "%OUTLINE_VAR%",    c.outlineVariant.name() },
        { "%INV_SURF%",       c.inverseSurface.name() },
        { "%ONINV_SURF%",     c.inverseOnSurface.name() },
    };
    for (const auto& s : subs)
        qss.replace(s.key, s.value);

    qApp->setStyleSheet(qss);
}
