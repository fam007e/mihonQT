#include "ExtensionManagerView.h"
#include "../source/ExtensionRepoManager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include "../source/ExtensionManager.h"

ExtensionManagerView::ExtensionManagerView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshAvailableExtensions();
}

void ExtensionManagerView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("Extensions", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #ECEFF4; padding: 10px;");
    layout->addWidget(titleLabel);

    // Repo management
    QHBoxLayout *repoLayout = new QHBoxLayout();
    m_repoUrlEdit = new QLineEdit(this);
    m_repoUrlEdit->setPlaceholderText("Enter repository URL...");
    m_repoUrlEdit->setStyleSheet("padding: 8px; background-color: #3B4252; color: white; border-radius: 4px;");
    
    QPushButton *addRepoBtn = new QPushButton("Add Repo", this);
    addRepoBtn->setStyleSheet("padding: 8px 15px; background-color: #88C0D0; color: #2E3440; font-weight: bold; border-radius: 4px;");
    connect(addRepoBtn, &QPushButton::clicked, this, &ExtensionManagerView::onAddRepoClicked);

    repoLayout->addWidget(m_repoUrlEdit);
    repoLayout->addWidget(addRepoBtn);
    layout->addLayout(repoLayout);

    // Available list
    layout->addWidget(new QLabel("Available Extensions:", this));
    m_availableList = new QListWidget(this);
    m_availableList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_availableList, &QListWidget::customContextMenuRequested, this, &ExtensionManagerView::onCustomContextMenuRequested);
    m_availableList->setStyleSheet(
        "QListWidget { background-color: #2E3440; border: none; border-radius: 4px; }"
        "QListWidget::item { padding: 10px; color: #ECEFF4; border-bottom: 1px solid #3B4252; }"
    );
    layout->addWidget(m_availableList);

    connect(m_availableList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        QString pkgName = item->data(Qt::UserRole).toString();
        QString downloadUrl = item->data(Qt::UserRole + 1).toString();
        
        if (QMessageBox::question(this, "Install Extension", "Do you want to install " + item->text() + "?") == QMessageBox::Yes) {
            // Basic download logic
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(downloadUrl)));
            
            connect(reply, &QNetworkReply::finished, this, [this, reply, pkgName]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
                    QString extensionsPath = dataDir + "/extensions/" + pkgName + ".js";
                    QDir().mkpath(dataDir + "/extensions");
                    
                    QFile file(extensionsPath);
                    if (file.open(QIODevice::WriteOnly)) { // flawfinder: ignore
                        file.write(reply->readAll());
                        file.close();
                        QMessageBox::information(this, "Success", "Extension installed. Restart recommended.");
                    }
                }
                reply->deleteLater();
            });
        }
    });
}

void ExtensionManagerView::onAddRepoClicked()
{
    QString url = m_repoUrlEdit->text().trimmed();
    if (!url.isEmpty()) {
        ExtensionRepoManager::instance().addRepo(url);
        m_repoUrlEdit->clear();
        refreshAvailableExtensions();
    }
}

void ExtensionManagerView::refreshAvailableExtensions()
{
    m_availableList->clear();
    auto repos = ExtensionRepoManager::instance().getRepos();
    
    for (const auto& repo : repos) {
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkRequest request(QUrl(repo.baseUrl + "/index.min.json"));
        QNetworkReply *reply = manager->get(request);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, repo]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonArray arr = doc.array();
                for (const QJsonValue& val : arr) {
                    QJsonObject obj = val.toObject();
                    QString name = obj["name"].toString();
                    QString pkg = obj["pkg"].toString();
                    // Assuming .js is in the same repo structure or construct URL
                    // Most repos use: baseUrl/scripts/pkg.js
                    QString downloadUrl = repo.baseUrl + "/scripts/" + pkg + ".js";

                    bool isTrusted = ExtensionManager::instance().isTrusted(pkg);
                    QString displayName = name + (isTrusted ? " [Trusted]" : "");
                    
                    QListWidgetItem *item = new QListWidgetItem(displayName, m_availableList);
                    item->setData(Qt::UserRole, pkg);
                    item->setData(Qt::UserRole + 1, downloadUrl);
                    if (isTrusted) {
                        item->setForeground(QColor("#A3BE8C")); // Greenish for trusted
                    }
                }
            }
            reply->deleteLater();
        });
    }
}

void ExtensionManagerView::onCustomContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem *item = m_availableList->itemAt(pos);
    if (!item) return;

    QString pkg = item->data(Qt::UserRole).toString();
    bool isTrusted = ExtensionManager::instance().isTrusted(pkg);

    QMenu menu(this);
    QAction *trustAction = menu.addAction(isTrusted ? "Untrust Extension" : "Trust Extension");
    
    QAction *selectedAction = menu.exec(m_availableList->mapToGlobal(pos));
    if (selectedAction == trustAction) {
        ExtensionManager::instance().setTrusted(pkg, !isTrusted);
        refreshAvailableExtensions(); // UI update
    }
}

void ExtensionManagerView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}
