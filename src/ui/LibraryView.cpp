#include "LibraryView.h"
#include "model/Manga.h"
#include "database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>

LibraryView::LibraryView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void LibraryView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_libraryListWidget = new QListWidget(this);
    m_libraryListWidget->setViewMode(QListWidget::IconMode);
    m_libraryListWidget->setIconSize(QSize(100, 150));
    m_libraryListWidget->setResizeMode(QListWidget::Adjust);
    m_libraryListWidget->setSpacing(10);

    layout->addWidget(m_libraryListWidget);

    connect(m_libraryListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int index = m_libraryListWidget->row(item);
        if (index >= 0 && index < m_libraryManga.size()) {
            emit mangaSelected(m_libraryManga[index]);
        }
    });
}

void LibraryView::refreshLibrary()
{
    m_libraryListWidget->clear();
    m_libraryManga.clear();

    MangaRepository repo(DatabaseManager::instance().database());
    m_libraryManga = repo.getFavorites();

    for (const Manga& manga : m_libraryManga) {
        QListWidgetItem *item = new QListWidgetItem(m_libraryListWidget);
        item->setText(manga.title());
        // Placeholder icon, in real app we would load from thumbnailUrl
        item->setIcon(QIcon::fromTheme("image-x-generic")); 
        m_libraryListWidget->addItem(item);
    }
}
