#include "SourceListView.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont> // For QFont
#include "source/SourceBase.h" // For SourceBase
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include "source/LocalSource.h"

SourceListView::SourceListView(SourceManager *sourceManager, QWidget *parent)
    : QWidget(parent)
    , m_sourceManager(sourceManager)
    , m_sourceListWidget(new QListWidget(this))
{
    setupUi();

    // Populate the list with sources from the SourceManager
    for (SourceBase* source : m_sourceManager->getAllSources()) {
        QListWidgetItem *item = new QListWidgetItem(source->name(), m_sourceListWidget);
        item->setData(Qt::UserRole, static_cast<qlonglong>(source->id())); // Store source ID in item's user data
    }

    // Connect itemClicked signal to emit sourceSelected signal
    connect(m_sourceListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
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
                    return; // User cancelled the dialog, do not proceed
                }
            }
        }
        emit sourceSelected(sourceId);
    });
}

void SourceListView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *titleLabel = new QLabel("Sources", this);
    titleLabel->setFont(QFont("Segoe UI", 16, QFont::Bold)); // Example font
    titleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addWidget(m_sourceListWidget);
}
