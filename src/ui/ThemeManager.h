#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum Theme {
        NordDark,
        NordLight,
        CatppuccinMocha,
        CatppuccinFrappe,
        TokyoNight,
        Dracula
    };
    Q_ENUM(Theme)

    static ThemeManager& instance();

    Theme currentTheme() const;

    void setAmoledMode(bool enabled);
    bool isAmoledMode() const;

public slots:
    void applyTheme(Theme theme);

signals:
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    Theme m_currentTheme;
    bool m_isAmoled;

    void applyNordDark();
    void applyNordLight();
    void applyCatppuccinMocha();
    void applyCatppuccinFrappe();
    void applyTokyoNight();
    void applyDracula();
};

#endif // THEMEMANAGER_H
