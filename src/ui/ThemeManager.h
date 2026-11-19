#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QString>

// Material 3 color tokens, mirroring Mihon's BaseColorScheme layout.
// secondary == unread badge color, tertiary == downloaded badge color.
struct M3Colors
{
    QColor primary, onPrimary, primaryContainer, onPrimaryContainer;
    QColor secondary, onSecondary, secondaryContainer, onSecondaryContainer;
    QColor tertiary, onTertiary, tertiaryContainer, onTertiaryContainer;
    QColor background, onBackground;
    QColor surface, onSurface;
    QColor surfaceVariant, onSurfaceVariant;
    QColor surfaceContainerLowest, surfaceContainerLow, surfaceContainer, surfaceContainerHigh, surfaceContainerHighest;
    QColor outline, outlineVariant;
    QColor error, onError, errorContainer, onErrorContainer;
    QColor inverseSurface, inverseOnSurface;
};

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    // NOTE: existing enum values are persisted in settings; only append.
    enum Theme {
        NordDark,
        NordLight,
        CatppuccinMocha,
        CatppuccinFrappe,
        TokyoNight,
        Dracula,
        MihonDefault
    };
    Q_ENUM(Theme)

    static ThemeManager& instance();

    Theme currentTheme() const;

    // Current M3 token set (re-read after every themeChanged).
    const M3Colors& colors() const { return m_colors; }
    bool isDarkTheme() const { return m_isDark; }
    bool isAmoledMode() const;

    void setAmoledMode(bool enabled);

    // ---- Native widget style (Breeze, Oxygen, Kvantum, Fusion, ...) ----
    //
    // By default the app draws itself with the Material 3 look: Fusion plus a
    // custom palette plus a global stylesheet. A stylesheet-drawn widget never
    // reaches the underlying QStyle, so a QStyle plugin can only take effect
    // when that stylesheet and palette are dropped. Native mode does exactly
    // that, which means the Material palettes / AMOLED option do not apply
    // while it is active.
    //
    // Style keys: empty string = Material (default); SystemStyleKey = whatever
    // Qt picked at launch (platform theme, -style, QT_STYLE_OVERRIDE); anything
    // else is a QStyleFactory key.
    static const QString SystemStyleKey;
    static QStringList availableNativeStyles();   // QStyleFactory::keys()
    QString nativeStyle() const { return m_nativeStyle; }
    bool isNativeStyle() const { return !m_nativeStyle.isEmpty(); }

    // Reads the persisted choice ("theme/style") without applying it; call
    // before the first applyTheme().
    void loadSavedStyle();
    // Applies and persists a style choice (see keys above).
    void setNativeStyle(const QString& styleKey);

    // Convenience: rgba() string from a token with an alpha multiplier.
    static QString rgba(const QColor& c, float alpha = 1.0f);

public slots:
    void applyTheme(Theme theme);

signals:
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    Theme m_currentTheme = MihonDefault;
    bool m_isAmoled = false;
    bool m_isDark = true;
    M3Colors m_colors;

    QString m_nativeStyle;          // see "Style keys" above
    QString m_systemStyleName;      // style Qt chose at launch
    QPalette m_systemPalette;       // palette Qt chose at launch

    void setTokens(const M3Colors& darkScheme, const M3Colors& lightScheme, bool darkMode);
    void setTokensFromPalette(const QPalette& pal);
    void applyAmoledOverride();
    void applyPalette();
    void applyGlobalQss();
    void applyNativeStyle();
};

#endif // THEMEMANAGER_H
