#include "ExtensionRepoManager.h"
#include "../config/PreferenceManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>

ExtensionRepoManager& ExtensionRepoManager::instance()
{
    static ExtensionRepoManager instance;
    return instance;
}

ExtensionRepoManager::ExtensionRepoManager()
{
    loadRepos();
}

ExtensionRepoManager::~ExtensionRepoManager()
{
}

void ExtensionRepoManager::loadRepos()
{
    QVariant data = PreferenceManager::instance().value("extension_repos");
    if (data.isValid()) {
        QJsonDocument doc = QJsonDocument::fromJson(data.toByteArray());
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            QJsonObject obj = val.toObject();
            ExtensionRepo repo;
            repo.name = obj["name"].toString();
            repo.baseUrl = obj["baseUrl"].toString();
            repo.website = obj["website"].toString();
            repo.shortName = obj["shortName"].toString();
            m_repos.append(repo);
        }
    }
}

void ExtensionRepoManager::saveRepos()
{
    QJsonArray arr;
    for (const auto& repo : m_repos) {
        QJsonObject obj;
        obj["name"] = repo.name;
        obj["baseUrl"] = repo.baseUrl;
        obj["website"] = repo.website;
        obj["shortName"] = repo.shortName;
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    PreferenceManager::instance().setValue("extension_repos", doc.toJson());
}

void ExtensionRepoManager::addRepo(const QString& baseUrl)
{
    // Check if exists
    for (const auto& r : m_repos) {
        if (r.baseUrl == baseUrl) return;
    }

    // Try to fetch metadata
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(baseUrl + "/index.min.json"));
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        // Here we just fetch the repo to validate it
        // Actually the index.min.json is for extensions, the repo meta might be elsewhere or just use baseUrl
        ExtensionRepo repo;
        repo.baseUrl = baseUrl;
        repo.name = baseUrl; // Fallback
        
        // Try to find if there's a meta info in index or adjacent?
        // Android version has separate meta. In most cases, it's just the URL.
        
        m_repos.append(repo);
        saveRepos();
        emit reposChanged();
        emit repoAdded(repo);
    } else {
        qWarning() << "Failed to add repo:" << reply->errorString();
    }
    reply->deleteLater();
}

void ExtensionRepoManager::removeRepo(const QString& baseUrl)
{
    for (int i = 0; i < m_repos.size(); ++i) {
        if (m_repos[i].baseUrl == baseUrl) {
            m_repos.removeAt(i);
            saveRepos();
            emit reposChanged();
            break;
        }
    }
}

QList<ExtensionRepo> ExtensionRepoManager::getRepos() const
{
    return m_repos;
}

void ExtensionRepoManager::refreshRepos()
{
    // Placeholder for async refresh of all repos
    emit refreshFinished();
}
