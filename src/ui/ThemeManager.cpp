#include "ThemeManager.h"
#include <QStyleFactory>

ThemeManager::ThemeManager(QObject *parent) : QObject(parent), m_currentTheme(NordDark), m_isAmoled(false)
{
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

void ThemeManager::setAmoledMode(bool enabled)
{
    if (m_isAmoled == enabled) return;
    m_isAmoled = enabled;
    applyTheme(m_currentTheme); // Re-apply current theme to update colors
}

bool ThemeManager::isAmoledMode() const
{
    return m_isAmoled;
}

void ThemeManager::applyTheme(Theme theme)
{
    m_currentTheme = theme;
    
    // Set Fusion style as a base for consistent cross-platform look
    qApp->setStyle(QStyleFactory::create("Fusion"));

    switch (theme) {
    case CatppuccinMocha: applyCatppuccinMocha(); break;
    case CatppuccinFrappe: applyCatppuccinFrappe(); break;
    case NordDark: applyNordDark(); break;
    case NordLight: applyNordLight(); break;
    case TokyoNight: applyTokyoNight(); break;
    case Dracula: applyDracula(); break;
    }

    emit themeChanged(theme);
}

void ThemeManager::applyCatppuccinMocha()
{
    QPalette p;
    p.setColor(QPalette::Window, m_isAmoled ? Qt::black : QColor("#1e1e2e"));
    p.setColor(QPalette::WindowText, QColor("#cdd6f4"));
    p.setColor(QPalette::Base, QColor("#181825"));
    p.setColor(QPalette::AlternateBase, QColor("#11111b"));
    p.setColor(QPalette::ToolTipBase, QColor("#181825"));
    p.setColor(QPalette::ToolTipText, QColor("#cdd6f4"));
    p.setColor(QPalette::Text, QColor("#cdd6f4"));
    p.setColor(QPalette::Button, QColor("#313244"));
    p.setColor(QPalette::ButtonText, QColor("#cdd6f4"));
    p.setColor(QPalette::BrightText, QColor("#f5e0dc"));
    p.setColor(QPalette::Link, QColor("#89b4fa"));
    p.setColor(QPalette::Highlight, QColor("#89b4fa"));
    p.setColor(QPalette::HighlightedText, QColor("#1e1e2e"));
    qApp->setPalette(p);
}

void ThemeManager::applyCatppuccinFrappe()
{
    QPalette p;
    p.setColor(QPalette::Window, m_isAmoled ? Qt::black : QColor("#303446"));
    p.setColor(QPalette::WindowText, QColor("#c6d0f5"));
    p.setColor(QPalette::Base, QColor("#292c3c"));
    p.setColor(QPalette::AlternateBase, QColor("#232634"));
    p.setColor(QPalette::ToolTipBase, QColor("#292c3c"));
    p.setColor(QPalette::ToolTipText, QColor("#c6d0f5"));
    p.setColor(QPalette::Text, QColor("#c6d0f5"));
    p.setColor(QPalette::Button, QColor("#414559"));
    p.setColor(QPalette::ButtonText, QColor("#c6d0f5"));
    p.setColor(QPalette::BrightText, QColor("#f2d5cf"));
    p.setColor(QPalette::Link, QColor("#8ca0dc"));
    p.setColor(QPalette::Highlight, QColor("#8ca0dc"));
    p.setColor(QPalette::HighlightedText, QColor("#303446"));
    qApp->setPalette(p);
}

void ThemeManager::applyNordDark()
{
    QPalette p;
    p.setColor(QPalette::Window, m_isAmoled ? Qt::black : QColor("#1f2430")); // Darkened Nord background
    p.setColor(QPalette::WindowText, QColor("#d8dee9"));
    p.setColor(QPalette::Base, QColor("#29303d")); // Darkened Nord base
    p.setColor(QPalette::AlternateBase, QColor("#3b4252"));
    p.setColor(QPalette::ToolTipBase, QColor("#29303d"));
    p.setColor(QPalette::ToolTipText, QColor("#d8dee9"));
    p.setColor(QPalette::Text, QColor("#eceff4"));
    p.setColor(QPalette::Button, QColor("#434c5e"));
    p.setColor(QPalette::ButtonText, QColor("#d8dee9"));
    p.setColor(QPalette::BrightText, QColor("#88c0d0"));
    p.setColor(QPalette::Link, QColor("#81a1c1"));
    p.setColor(QPalette::Highlight, QColor("#88c0d0"));
    p.setColor(QPalette::HighlightedText, QColor("#1f2430"));
    qApp->setPalette(p);
}

void ThemeManager::applyNordLight()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor("#eceff4"));
    p.setColor(QPalette::WindowText, QColor("#2e3440"));
    p.setColor(QPalette::Base, QColor("#e5e9f0"));
    p.setColor(QPalette::AlternateBase, QColor("#d8dee9"));
    p.setColor(QPalette::ToolTipBase, QColor("#e5e9f0"));
    p.setColor(QPalette::ToolTipText, QColor("#2e3440"));
    p.setColor(QPalette::Text, QColor("#2e3440"));
    p.setColor(QPalette::Button, QColor("#d8dee9"));
    p.setColor(QPalette::ButtonText, QColor("#2e3440"));
    p.setColor(QPalette::BrightText, QColor("#88c0d0"));
    p.setColor(QPalette::Link, QColor("#5e81ac"));
    p.setColor(QPalette::Highlight, QColor("#88c0d0"));
    p.setColor(QPalette::HighlightedText, QColor("#eceff4"));
    qApp->setPalette(p);
}

void ThemeManager::applyTokyoNight()
{
    QPalette p;
    p.setColor(QPalette::Window, m_isAmoled ? Qt::black : QColor("#1a1b26"));
    p.setColor(QPalette::WindowText, QColor("#a9b1d6"));
    p.setColor(QPalette::Base, QColor("#24283b"));
    p.setColor(QPalette::AlternateBase, QColor("#1f2335"));
    p.setColor(QPalette::ToolTipBase, QColor("#24283b"));
    p.setColor(QPalette::ToolTipText, QColor("#a9b1d6"));
    p.setColor(QPalette::Text, QColor("#c0caf5"));
    p.setColor(QPalette::Button, QColor("#414868"));
    p.setColor(QPalette::ButtonText, QColor("#a9b1d6"));
    p.setColor(QPalette::BrightText, QColor("#f7768e"));
    p.setColor(QPalette::Link, QColor("#7aa2f7"));
    p.setColor(QPalette::Highlight, QColor("#7aa2f7"));
    p.setColor(QPalette::HighlightedText, QColor("#1a1b26"));
    qApp->setPalette(p);
}

void ThemeManager::applyDracula()
{
    QPalette p;
    p.setColor(QPalette::Window, m_isAmoled ? Qt::black : QColor("#282a36"));
    p.setColor(QPalette::WindowText, QColor("#f8f8f2"));
    p.setColor(QPalette::Base, QColor("#44475a"));
    p.setColor(QPalette::AlternateBase, QColor("#282a36"));
    p.setColor(QPalette::ToolTipBase, QColor("#44475a"));
    p.setColor(QPalette::ToolTipText, QColor("#f8f8f2"));
    p.setColor(QPalette::Text, QColor("#f8f8f2"));
    p.setColor(QPalette::Button, QColor("#44475a"));
    p.setColor(QPalette::ButtonText, QColor("#f8f8f2"));
    p.setColor(QPalette::BrightText, QColor("#ff79c6"));
    p.setColor(QPalette::Link, QColor("#8be9fd"));
    p.setColor(QPalette::Highlight, QColor("#bd93f9"));
    p.setColor(QPalette::HighlightedText, QColor("#282a36"));
    qApp->setPalette(p);
}
