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

    void setTokens(const M3Colors& darkScheme, const M3Colors& lightScheme, bool darkMode);
    void applyAmoledOverride();
    void applyPalette();
    void applyGlobalQss();
};

#endif // THEMEMANAGER_H
