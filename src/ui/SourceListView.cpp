#include "SourceListView.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QLabel>
#include <QFont>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QPushButton>
#include "UiUtils.h"
#include "source/SourceBase.h"
#include "../source/LocalSource.h"

SourceListView::SourceListView(SourceManager *sourceManager, QWidget *parent)
    : QWidget(parent)
    , m_sourceManager(sourceManager)
    , m_sourceListWidget(new QListWidget(this))
{
    setupUi();
    refreshSources();

    UiUtils::applyListWidgetStyle(m_sourceListWidget);
    m_sourceListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_sourceListWidget, &QListWidget::itemClicked, this, &SourceListView::onSourceClicked);
    connect(m_sourceListWidget, &QListWidget::customContextMenuRequested, this, &SourceListView::onCustomContextMenuRequested);
}

void SourceListView::refreshSources()
{
    m_sourceListWidget->clear();
    // Populate the list with sources from the SourceManager
    for (SourceBase* source : m_sourceManager->getAllSources()) {
        QListWidgetItem *item = new QListWidgetItem(source->name(), m_sourceListWidget);
        item->setData(Qt::UserRole, static_cast<qlonglong>(source->id())); // Store source ID in item's user data
    }
}

void SourceListView::onSourceClicked(QListWidgetItem *item)
{
    if (!item) return;
    long sourceId = item->data(Qt::UserRole).toLongLong();

    SourceBase* source = m_sourceManager->getSourceById(sourceId);
    if (LocalSource* localSource = dynamic_cast<LocalSource*>(source)) {
        if (localSource->getBaseDirectory().isEmpty()) {
            QString dir = QFileDialog::getExistingDirectory(this, "Select Local Manga Directory", QDir::homePath());
            if (!dir.isEmpty()) {
                localSource->setBaseDirectory(dir);
                QSettings settings("MihonQT", "MihonQT");
                settings.setValue("localMangaPath", dir);
            } else {
                return; // User cancelled
            }
        }
    }
    emit sourceSelected(sourceId);
}

void SourceListView::onCustomContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = m_sourceListWidget->itemAt(pos);
    if (!item) return;

    long sourceId = item->data(Qt::UserRole).toLongLong();
    SourceBase* source = m_sourceManager->getSourceById(sourceId);

    if (source && source->id() == 0) { // Local Source ID is 0
        QMenu contextMenu(this);
        QAction *editPathAction = contextMenu.addAction("Edit Local Manga Folder");

        connect(editPathAction, &QAction::triggered, this, [this, source]() {
            QString dir = QFileDialog::getExistingDirectory(this, "Select Local Manga Directory",
                                                          QDir::homePath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            if (!dir.isEmpty()) {
                QSettings settings("MihonQT", "MihonQT");
                settings.setValue("localMangaPath", dir);

                // Cast to LocalSource and update path
                LocalSource* localSource = dynamic_cast<LocalSource*>(source);
                if (localSource) {
                    localSource->setBaseDirectory(dir);
                    QMessageBox::information(this, "Success", "Local manga path updated to:\n" + dir);
                }
            }
        });

        contextMenu.exec(m_sourceListWidget->viewport()->mapToGlobal(pos));
    }
}

void SourceListView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *titleLabel = new QLabel("Sources", this);
    titleLabel->setFont(QFont("Inter", 16, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("padding: 20px; color: #ECEFF4;");

    layout->addWidget(titleLabel);
    layout->addWidget(m_sourceListWidget);
}
