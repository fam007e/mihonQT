#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "MainWindow.h"
#include "database/DatabaseManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Mihon");
    app.setApplicationName("MihonQT");
    qDebug() << "MihonQT starting. Build Date:" << __DATE__ << "Time:" << __TIME__;

    // Initialize the database
    if (!DatabaseManager::instance().openDatabase()) {
        QMessageBox::critical(nullptr, "Database Error",
                              "Could not open the database. The application will now exit.");
        return 1;
    }

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
