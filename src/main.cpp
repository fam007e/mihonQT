#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QtGlobal>
#include <QWebEngineProfile>
#include <QRegularExpression>
#include "MainWindow.h"
#include "database/DatabaseManager.h"
#include "network/NetworkAccessManager.h"

int main(int argc, char *argv[])
{
    // Modern Chromium partitions third-party storage, which denies the
    // Cloudflare Turnstile iframe the writes it needs inside the challenge
    // WebView ("requestStorageAccess: Permission denied") and the challenge
    // can never complete. Disable partitioning and StorageAccessAPI so
    // Turnstile does not fail on permission-denied storage requests.
    if (qEnvironmentVariableIsEmpty("QTWEBENGINE_CHROMIUM_FLAGS")) {
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-features=ThirdPartyStoragePartitioning,StorageAccessAPI");
    }

    QApplication app(argc, argv);
    app.setApplicationName("MihonQT");
    qDebug() << "MihonQT starting. Build Date:" << __DATE__ << "Time:" << __TIME__;

    // Establish a unified browser identity early:
    // Sync the WebEngine profile and NetworkAccessManager to the native Chromium UA
    // with QtWebEngine stripped, ensuring initial extension HTTP requests and
    // challenge dialogs present the exact same UA and client hints.
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QString ua = profile->httpUserAgent();
    static const QRegularExpression qtToken("QtWebEngine/[\\d.]+\\s*;?\\s*");
    ua.remove(qtToken);
    profile->setHttpUserAgent(ua);
    NetworkAccessManager::setUserAgentOverride(ua);

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
