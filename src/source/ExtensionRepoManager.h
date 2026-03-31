#ifndef EXTENSIONREPOMANAGER_H
#define EXTENSIONREPOMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QUrl>

struct ExtensionRepo {
    QString name;
    QString baseUrl;
    QString website;
    QString shortName;
};

class ExtensionRepoManager : public QObject
{
    Q_OBJECT
public:
    static ExtensionRepoManager& instance();

    void addRepo(const QString& baseUrl);
    void removeRepo(const QString& baseUrl);
    QList<ExtensionRepo> getRepos() const;

    void refreshRepos();

signals:
    void reposChanged();
    void repoAdded(const ExtensionRepo& repo);
    void refreshFinished();

private:
    ExtensionRepoManager();
    ~ExtensionRepoManager();
    void loadRepos();
    void saveRepos();

    QList<ExtensionRepo> m_repos;
};

#endif // EXTENSIONREPOMANAGER_H
