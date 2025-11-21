#include "MangaDetailsView.h"
#include "database/DatabaseManager.h"
#include "database/MangaRepository.h"
#include "database/ChapterRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

MangaDetailsView::MangaDetailsView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void MangaDetailsView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header: Back button + Title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("Back", this);
    connect(backButton, &QPushButton::clicked, this, &MangaDetailsView::backRequested);
    headerLayout->addWidget(backButton);
    
    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(16);
    m_titleLabel->setFont(titleFont);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Info area
    m_authorLabel = new QLabel(this);
    mainLayout->addWidget(m_authorLabel);

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(m_descriptionLabel);

    // Action buttons
    m_libraryButton = new QPushButton(this);
    connect(m_libraryButton, &QPushButton::clicked, this, [this]() {
        MangaRepository repo(DatabaseManager::instance().database());
        m_manga.setFavorite(!m_manga.favorite());
        if (repo.updateManga(m_manga)) {
            updateLibraryButton();
            emit libraryStatusChanged();
        } else {
            QMessageBox::warning(this, "Error", "Failed to update library status.");
            // Revert local change if DB update failed
            m_manga.setFavorite(!m_manga.favorite());
        }
    });
    mainLayout->addWidget(m_libraryButton);

    // Chapter List
    m_chapterListWidget = new QListWidget(this);
    connect(m_chapterListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int index = m_chapterListWidget->row(item);
        if (index >= 0 && index < m_chapters.size()) {
            emit chapterSelected(m_manga, m_chapters[index]);
        }
    });
    mainLayout->addWidget(m_chapterListWidget);
}

void MangaDetailsView::setManga(const Manga& manga)
{
    m_manga = manga;
    m_titleLabel->setText(m_manga.title());
    m_authorLabel->setText("Author: " + m_manga.author());
    m_descriptionLabel->setText(m_manga.description());
    updateLibraryButton();
    refreshChapters();
}

void MangaDetailsView::updateLibraryButton()
{
    if (m_manga.favorite()) {
        m_libraryButton->setText("Remove from Library");
    } else {
        m_libraryButton->setText("Add to Library");
    }
}

void MangaDetailsView::refreshChapters()
{
    m_chapterListWidget->clear();
    m_chapters.clear();

    ChapterRepository repo(DatabaseManager::instance().database());
    m_chapters = repo.getChaptersByMangaId(m_manga.id());

    for (const Chapter& chapter : m_chapters) {
        QListWidgetItem *item = new QListWidgetItem(m_chapterListWidget);
        item->setText(QString("Chapter %1: %2").arg(chapter.chapterNumber()).arg(chapter.name()));
        m_chapterListWidget->addItem(item);
    }
}
