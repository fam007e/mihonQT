#include <QTest>
#include <QApplication>
#include <QProxyStyle>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QStyle>
#include <QStyleFactory>
#include <QTemporaryDir>

#include "ui/ThemeManager.h"
#include "config/PreferenceManager.h"

// Native widget style mode: Breeze/Oxygen/Kvantum/etc. only take effect when
// the app-wide Material stylesheet and palette are dropped. Fusion and Windows
// ship with every Qt build, so they stand in for the plugin styles here.
class TestThemeStyle : public QObject
{
    Q_OBJECT

    QString m_launchStyle;
    QPalette m_launchPalette;
    QVariant m_savedStyleSetting; // the developer's real value, restored on exit

    static bool hasStyle(const QString& key)
    {
        for (const QString& k : QStyleFactory::keys())
            if (k.compare(key, Qt::CaseInsensitive) == 0) return true;
        return false;
    }

    // Native mode keeps no application stylesheet at all (any non-empty one
    // makes Qt wrap every widget in QStyleSheetStyle), but the applied style
    // is ThemeManager's QProxyStyle wrapper, whose own name() is empty —
    // QStyle::name() only reports what QStyleFactory recorded. Read the
    // wrapped base style instead.
    static QString appliedBaseStyleName()
    {
        const auto *proxy = qobject_cast<const QProxyStyle *>(QApplication::style());
        return proxy ? proxy->baseStyle()->name().toLower() : QString();
    }

    static bool materialSheetActive()
    {
        return qApp->styleSheet().contains("QWidget { background-color");
    }

private slots:
    void initTestCase()
    {
        // main() redirects QSettings on file-backed platforms, but the native
        // format is the registry on Windows and plist on macOS, which setPath()
        // cannot redirect. Snapshot the real value so no platform ever ends up
        // with a changed saved style.
        m_savedStyleSetting = PreferenceManager::instance().value("theme/style");

        // Capture before ThemeManager exists, exactly as it does itself.
        m_launchStyle = QApplication::style()->name();
        m_launchPalette = QApplication::palette();
        if (!hasStyle("Windows") || !hasStyle("Fusion"))
            QSKIP("Windows/Fusion styles not available in this Qt build");
    }

    void cleanupTestCase()
    {
        PreferenceManager::instance().setValue(
            "theme/style", m_savedStyleSetting.isValid() ? m_savedStyleSetting : QVariant(QString()));
    }

    void init()
    {
        // Singleton: put it back to the Material default before every test.
        ThemeManager::instance().setNativeStyle(QString());
        ThemeManager::instance().applyTheme(ThemeManager::MihonDefault);
        ThemeManager::instance().setAmoledMode(false);
    }

    void materialIsTheDefault()
    {
        ThemeManager& tm = ThemeManager::instance();
        QVERIFY(!tm.isNativeStyle());
        QVERIFY(!qApp->styleSheet().isEmpty());
        QCOMPARE(tm.colors().background, QColor(0x1B1B1F));
    }

    void nativeStyleDropsStylesheetAndPalette()
    {
        ThemeManager& tm = ThemeManager::instance();
        // A QSignalSpy rather than a reference-capturing lambda: when a
        // QVERIFY below fails it returns early, and a surviving connection
        // with a dead stack-frame reference corrupts memory on the next emit.
        QSignalSpy spy(&tm, &ThemeManager::themeChanged);

        tm.setNativeStyle("windows");

        QVERIFY(tm.isNativeStyle());
        QCOMPARE(tm.nativeStyle(), QString("Windows")); // factory spelling
        QVERIFY(!materialSheetActive());
        QCOMPARE(appliedBaseStyleName(), QString("windows"));
        QCOMPARE(QApplication::palette().color(QPalette::Window),
                 m_launchPalette.color(QPalette::Window));
        QVERIFY(spy.count() >= 1);

        // A widget made now is drawn by the native style, not stylesheet-wrapped.
        QPushButton button("x");
        QVERIFY(!button.style()->inherits("QStyleSheetStyle"));
        QCOMPARE(button.style(), QApplication::style());
    }

    void tokensFollowThePalette()
    {
        ThemeManager& tm = ThemeManager::instance();
        const QColor materialBg = tm.colors().background;
        tm.setNativeStyle("fusion");

        QVERIFY(tm.colors().background != materialBg);
        QCOMPARE(tm.colors().background, m_launchPalette.color(QPalette::Window));
        QCOMPARE(tm.colors().onBackground, m_launchPalette.color(QPalette::WindowText));
        QCOMPARE(tm.colors().primary, m_launchPalette.color(QPalette::Highlight));
        QCOMPARE(tm.isDarkTheme(), m_launchPalette.color(QPalette::Window).lightness() < 128);
    }

    void amoledAndThemeAreInertInNativeMode()
    {
        ThemeManager& tm = ThemeManager::instance();
        tm.setNativeStyle("fusion");
        const QColor bg = tm.colors().background;

        tm.setAmoledMode(true);
        tm.applyTheme(ThemeManager::Dracula);

        QVERIFY(!materialSheetActive());
        QCOMPARE(tm.colors().background, bg);
    }

    void systemDefaultRestoresLaunchStyle()
    {
        ThemeManager& tm = ThemeManager::instance();
        tm.setNativeStyle("windows");
        tm.setNativeStyle(ThemeManager::SystemStyleKey);

        QCOMPARE(tm.nativeStyle(), ThemeManager::SystemStyleKey);
        QCOMPARE(appliedBaseStyleName(), m_launchStyle.toLower());
        QVERIFY(!materialSheetActive());
    }

    void leavingNativeModeRestoresMaterial()
    {
        ThemeManager& tm = ThemeManager::instance();
        tm.setNativeStyle("windows");
        tm.setNativeStyle(QString());

        QVERIFY(!tm.isNativeStyle());
        QVERIFY(!qApp->styleSheet().isEmpty());
        QCOMPARE(tm.colors().background, QColor(0x1B1B1F));
    }

    void unknownStyleFallsBackToMaterial()
    {
        ThemeManager& tm = ThemeManager::instance();
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression("not installed; using Material"));
        tm.setNativeStyle("definitely-not-installed");

        QVERIFY(!tm.isNativeStyle());
        QVERIFY(!qApp->styleSheet().isEmpty());
    }

    void choicePersistsAndReloads()
    {
        ThemeManager& tm = ThemeManager::instance();
        tm.setNativeStyle("fusion"); // case-insensitive input
        QCOMPARE(PreferenceManager::instance().value("theme/style").toString(),
                 QString("Fusion"));

        // What the next launch does: read the saved key, then apply.
        PreferenceManager::instance().setValue("theme/style", "Windows");
        tm.loadSavedStyle();
        QCOMPARE(tm.nativeStyle(), QString("Windows"));
        tm.applyTheme(ThemeManager::MihonDefault);
        QVERIFY(!materialSheetActive());
        QCOMPARE(appliedBaseStyleName(), QString("windows"));

        tm.setNativeStyle(QString());
        QVERIFY(PreferenceManager::instance().value("theme/style").toString().isEmpty());
    }
};

int main(int argc, char *argv[])
{
    // Needs no display server, and should not touch the developer's real
    // settings: PreferenceManager writes through QSettings(org, app), which is
    // always NativeFormat (setDefaultFormat() does not apply to that
    // constructor). See initTestCase() for the non-file-backed platforms.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    QTemporaryDir settingsDir;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsDir.path());

    QApplication app(argc, argv);
    TestThemeStyle test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_theme_style.moc"
