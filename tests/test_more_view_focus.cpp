#include <QTest>
#include <QApplication>
#include <QCheckBox>
#include <QFocusEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include "ui/MoreView.h"

// Keyboard focus on the More screen's rows. The button rows used to have their
// whole stylesheet replaced on every focus change, dropping `border: none`.
// With the Material global stylesheet that was masked; with a native widget
// style (Breeze, Kvantum, ...) every focused row grew a visible button frame.
class TestMoreViewFocus : public QObject
{
    Q_OBJECT

    static void sendFocus(QWidget* w, QEvent::Type type)
    {
        QFocusEvent e(type, Qt::TabFocusReason);
        QApplication::sendEvent(w, &e);
    }

private slots:
    void focusChangeKeepsButtonRowStylesheet()
    {
        MoreView view;
        const QList<QPushButton*> buttons = view.findChildren<QPushButton*>();
        QVERIFY2(!buttons.isEmpty(), "More screen should have button rows");

        for (QPushButton* btn : buttons) {
            const QString before = btn->styleSheet();
            QVERIFY(before.contains("border: none"));

            sendFocus(btn, QEvent::FocusIn);
            QCOMPARE(btn->styleSheet(), before);
            sendFocus(btn, QEvent::FocusOut);
            QCOMPARE(btn->styleSheet(), before);
        }
    }

    void toggleRowsStillGetFocusHighlight()
    {
        MoreView view;
        const QList<QCheckBox*> toggles = view.findChildren<QCheckBox*>();
        QVERIFY2(!toggles.isEmpty(), "More screen should have toggle rows");

        for (QCheckBox* toggle : toggles) {
            QWidget* row = toggle->parentWidget(); // the focusable row container
            QVERIFY(row);
            const QString idle = row->styleSheet();

            sendFocus(row, QEvent::FocusIn);
            QVERIFY2(row->styleSheet() != idle, "focused toggle row is highlighted");

            sendFocus(row, QEvent::FocusOut);
            QCOMPARE(row->styleSheet(), QString("background-color: transparent;"));
        }
    }
};

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    QTemporaryDir settingsDir; // never touch the developer's real settings
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsDir.path());

    QApplication app(argc, argv);
    TestMoreViewFocus test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_more_view_focus.moc"
