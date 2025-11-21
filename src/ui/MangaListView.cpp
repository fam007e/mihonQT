#include "MangaListView.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include "model/Manga.h" // For Manga class

MangaListView::MangaListView(QWidget *parent)
    : QWidget(parent)
    , m_mangaListWidget(new QListWidget(this))
{
    setupUi();

    // Connect itemClicked signal to emit mangaSelected signal
    connect(m_mangaListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int row = m_mangaListWidget->row(item);
        if (row >= 0 && row < m_mangaList.size()) {
            emit mangaSelected(m_mangaList.at(row));
        }
    });
}

void MangaListView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *titleLabel = new QLabel("Manga List", this);
    titleLabel->setFont(QFont("Segoe UI", 16, QFont::Bold)); // Example font
    titleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addWidget(m_mangaListWidget);
}

void MangaListView::populateManga(const QList<Manga>& mangaList)
{
    m_mangaList = mangaList; // Store the manga list
    m_mangaListWidget->clear(); // Clear existing items
    for (const Manga& manga : m_mangaList) {
        QListWidgetItem *item = new QListWidgetItem(manga.title(), m_mangaListWidget);
        // We no longer need to store the ID here, we use the row index
    }
}
