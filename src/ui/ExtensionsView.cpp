#include "ExtensionsView.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QIcon>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QStandardPaths>
#include <QInputDialog>
#include <QMessageBox>
#include "MainWindow.h" // To get data directory or manage extension reloads

ExtensionsView::ExtensionsView(QWidget *parent)
    : QWidget(parent)
    , m_extensionManager(ExtensionManager::instance())
{
    setupUi();
    connect(&m_extensionManager, &ExtensionManager::availableExtensionsChanged, this, &ExtensionsView::refreshList);
}

void ExtensionsView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    // Header
    QHBoxLayout *header = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("Back", this);
    connect(backBtn, &QPushButton::clicked, this, &ExtensionsView::backRequested);
    header->addWidget(backBtn);

    QLabel *title = new QLabel("Extensions", this);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #ECEFF4;");
    header->addWidget(title);
    header->addStretch();

    QPushButton *addRepoBtn = new QPushButton("Add Repo", this);
    connect(addRepoBtn, &QPushButton::clicked, this, &ExtensionsView::addRepository);
    header->addWidget(addRepoBtn);

    layout->addLayout(header);

    // Search bar
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search extensions...");
    m_searchEdit->setStyleSheet("background-color: #2E3440; border: 1px solid #4C566A; padding: 10px; color: #ECEFF4;");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ExtensionsView::refreshList);
    layout->addWidget(m_searchEdit);

    // Lists in scroll area
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: transparent;");

    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setAlignment(Qt::AlignTop);

    QLabel *availableHeader = new QLabel("Available Extensions", this);
    availableHeader->setStyleSheet("font-size: 16px; font-weight: bold; color: #88C0D0; margin-top: 10px;");
    contentLayout->addWidget(availableHeader);

    m_availableList = new QListWidget(this);
    m_availableList->setFrameShape(QFrame::NoFrame);
    m_availableList->setStyleSheet("QListWidget { background-color: transparent; outline: 0; }");
    m_availableList->setSpacing(4);
    contentLayout->addWidget(m_availableList);

    scroll->setWidget(content);
    layout->addWidget(scroll);
}

void ExtensionsView::refreshList()
{
    m_availableList->clear();
    
    QString searchTerm = m_searchEdit->text().toLower();
    QSettings settings("MihonQT", "MihonQT");
    QString defaultDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString dataPath = settings.value("dataDirectory", defaultDataDir).toString();
    QString extensionsDir = dataPath + "/extensions";

    auto exts = m_extensionManager.availableExtensions();
    for (const auto& ext : exts) {
        if (!searchTerm.isEmpty() && !ext.name.toLower().contains(searchTerm)) continue;

        QListWidgetItem *item = new QListWidgetItem(m_availableList);
        item->setSizeHint(QSize(0, 70));

        QWidget *widget = new QWidget();
        QHBoxLayout *h = new QHBoxLayout(widget);
        h->setContentsMargins(10, 10, 10, 10);
        
        QLabel *icon = new QLabel();
        // Here you'd fetch the icon if missing, but let's just use a placeholder for now
        icon->setFixedSize(48, 48);
        icon->setStyleSheet("background-color: #3B4252; border-radius: 4px;");

        QVBoxLayout *v = new QVBoxLayout();
        v->setSpacing(2);
        QLabel *name = new QLabel(ext.name);
        name->setStyleSheet("font-weight: bold; color: #ECEFF4;");
        QLabel *info = new QLabel(QString("%1 • %2").arg(ext.pkg).arg(ext.lang));
        info->setStyleSheet("color: #D8DEE9; font-size: 11px;");
        v->addWidget(name);
        v->addWidget(info);

        QPushButton *installBtn = new QPushButton("Install");
        if (m_extensionManager.isInstalled(ext.pkg, extensionsDir)) {
            installBtn->setText("Installed");
            installBtn->setEnabled(false);
        }

        connect(installBtn, &QPushButton::clicked, this, [this, ext, extensionsDir, installBtn]() {
            installBtn->setEnabled(false);
            installBtn->setText("Installing...");
            
            // We pass null for NetworkAccessManager as the current singleton 
            // doesn't have an instance easily accessible here, but ExtensionManager
            // uses its own internally in this implementation for simplicity.
            m_extensionManager.installExtension(ext, nullptr, extensionsDir);
        });

        h->addWidget(icon);
        h->addLayout(v);
        h->addStretch();
        h->addWidget(installBtn);

        m_availableList->addItem(item);
        m_availableList->setItemWidget(item, widget);
    }
}

void ExtensionsView::addRepository()
{
    bool ok;
    QString url = QInputDialog::getText(this, "Add Repository", 
                                        "Enter Repository Index URL (e.g., https://.../repo):", 
                                        QLineEdit::Normal, "", &ok);
    if (ok && !url.isEmpty()) {
        m_extensionManager.addRepository(url);
        // Refresh
        m_extensionManager.fetchAvailableExtensions(nullptr);
    }
}
