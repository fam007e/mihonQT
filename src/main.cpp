#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QtGlobal>
#include "MainWindow.h"
#include "database/DatabaseManager.h"

int main(int argc, char *argv[])
{
    // Modern Chromium partitions third-party storage, which denies the
    // Cloudflare Turnstile iframe the writes it needs inside the challenge
    // WebView ("requestStorageAccess: Permission denied") and the challenge
    // can never complete. Disable partitioning unless the user has chosen
    // their own Chromium flags. Must be set before QApplication starts.
    if (qEnvironmentVariableIsEmpty("QTWEBENGINE_CHROMIUM_FLAGS")) {
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-features=ThirdPartyStoragePartitioning");
    }

    QApplication app(argc, argv);
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
