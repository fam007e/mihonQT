#include <QTest>
#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QSettings>
#include <QTemporaryDir>

#include "ui/StatisticsView.h"
#include "database/DatabaseManager.h"

class TestStatisticsView : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QVERIFY(DatabaseManager::instance().openDatabase(":memory:"));
    }

    // Unparented-to-a-layout child widgets are not laid out by Qt: they pile up
    // at (0,0) of the view. StatisticsView once created four such "0" labels,
    // which showed as a stray "0" in the top-left corner of the screen.
    void everyDirectChildLabelIsInTheLayout()
    {
        StatisticsView view;
        QVERIFY(view.layout());

        const QList<QLabel*> labels =
            view.findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY2(!labels.isEmpty(), "the title label is a direct child");

        for (QLabel* label : labels) {
            QVERIFY2(view.layout()->indexOf(label) >= 0,
                     qPrintable(QString("label \"%1\" is not managed by the layout")
                                    .arg(label->text())));
        }
    }

    // Dashboard sections: Library (4 cards) + Reading state (3) + Activity (3)
    // + Outside library (3) = 13 value labels, all keyed for refreshStats.
    void showsDashboardStatCards()
    {
        StatisticsView view;
        QCOMPARE(view.findChildren<QLabel*>("valueLabel").size(), 13);
        QCOMPARE(view.findChildren<QLabel*>("sectionTitle").size(), 4);
    }
};

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");
    QTemporaryDir settingsDir; // never touch the developer's real settings
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsDir.path());

    QApplication app(argc, argv);
    TestStatisticsView test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_statistics_view.moc"
