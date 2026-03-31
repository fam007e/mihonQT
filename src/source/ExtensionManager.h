#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>

class NetworkAccessManager;

struct RemoteExtension {
    QString name;
    QString pkg;
    QString apk; // This would be the .js filename in our case
    QString lang;
    long code;
    QString version;
    bool nsfw;
    QString iconUrl;
    QString repoUrl;
    
    // In our implementation, we'll download the JS file
    QString getJsUrl() const { return repoUrl + "/apk/" + apk; }
};

class ExtensionManager : public QObject
{
    Q_OBJECT
public:
    static ExtensionManager& instance();

    void addRepository(const QString& url);
    void removeRepository(const QString& url);
    QList<QString> getRepositories() const;

    void fetchAvailableExtensions(NetworkAccessManager* networkManager);
    QList<RemoteExtension> availableExtensions() const { return m_availableExtensions; }

    void installExtension(const RemoteExtension& ext, NetworkAccessManager* networkManager, const QString& extensionsDir);
    bool isInstalled(const QString& pkgName, const QString& extensionsDir) const;

signals:
    void availableExtensionsChanged();
    void extensionInstalled(const QString& pkgName);
    void errorOccurred(const QString& error);

private:
    ExtensionManager();
    ~ExtensionManager();
    ExtensionManager(const ExtensionManager&) = delete;
    ExtensionManager& operator=(const ExtensionManager&) = delete;

    void parseIndex(const QByteArray& data, const QString& repoUrl);

    QList<QString> m_repositories;
    QList<RemoteExtension> m_availableExtensions;
};

#endif // EXTENSIONMANAGER_H
