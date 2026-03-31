#include "MangaDetailsView.h"
#include "database/DatabaseManager.h"
#include "database/MangaRepository.h"
#include "database/ChapterRepository.h"
#include "database/CategoryRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileInfo>
#include "UiUtils.h"
#include "WebViewDialog.h"

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
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ECEFF4; padding-left: 10px;");
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    
    m_openWebViewButton = new QPushButton(this);
    m_openWebViewButton->setIcon(QIcon(":/icons/public.svg"));
    m_openWebViewButton->setToolTip("Open in WebView");
    m_openWebViewButton->setFlat(true);
    connect(m_openWebViewButton, &QPushButton::clicked, this, [this]() {
        QString fullUrl = m_manga.url();
        if (!fullUrl.isEmpty()) {
            WebViewDialog dialog(fullUrl, this);
            dialog.exec();
        }
    });
    headerLayout->addWidget(m_openWebViewButton);
    mainLayout->addLayout(headerLayout);

    // Info area
    m_authorLabel = new QLabel(this);
    mainLayout->addWidget(m_authorLabel);

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(m_descriptionLabel);

    // Action buttons
    QHBoxLayout *actionsLayout = new QHBoxLayout();

    m_libraryButton = new QPushButton(this);
    connect(m_libraryButton, &QPushButton::clicked, this, [this]() {
        MangaRepository& repo = DatabaseManager::instance().mangaRepository();
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
    actionsLayout->addWidget(m_libraryButton);

    m_editCategoriesButton = new QPushButton("Edit Categories", this);
    connect(m_editCategoriesButton, &QPushButton::clicked, this, [this]() {
        if (!m_manga.favorite()) {
            QMessageBox::information(this, "Info", "Add to library first to set categories.");
            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle("Set Categories");
        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        CategoryRepository& catRepo = DatabaseManager::instance().categoryRepository();
        QList<Category> allCategories = catRepo.getAllCategories();
        QList<long> currentCatIds = catRepo.getCategoriesForManga(m_manga.id());

        QList<QCheckBox*> checkBoxes;
        for (const Category& cat : allCategories) {
            QCheckBox *cb = new QCheckBox(cat.name(), &dialog);
            cb->setProperty("categoryId", static_cast<qlonglong>(cat.id()));
            if (currentCatIds.contains(cat.id())) {
                cb->setChecked(true);
            }
            layout->addWidget(cb);
            checkBoxes.append(cb);
        }

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttonBox);

        if (dialog.exec() == QDialog::Accepted) {
            QList<long> newCatIds;
            for (QCheckBox *cb : checkBoxes) {
                if (cb->isChecked()) {
                    newCatIds.append(cb->property("categoryId").toLongLong());
                }
            }
            catRepo.setCategoriesForManga(m_manga.id(), newCatIds);
            emit libraryStatusChanged(); // Refresh library to show in new tabs
        }
    });
    actionsLayout->addWidget(m_editCategoriesButton);

    mainLayout->addLayout(actionsLayout);

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

    ChapterRepository& repo = DatabaseManager::instance().chapterRepository();
    m_chapters = repo.getChaptersByMangaId(m_manga.id());

    for (const Chapter& chapter : m_chapters) {
        QListWidgetItem *item = new QListWidgetItem(m_chapterListWidget);
        QString displayName = chapter.name();

        // Remove file extensions if present
        if (displayName.endsWith(".cbz", Qt::CaseInsensitive) ||
            displayName.endsWith(".zip", Qt::CaseInsensitive)) {
            displayName = QFileInfo(displayName).completeBaseName();
        }

        // If the name is just the number or "Chapter X", avoid duplicating it
        // The user suggested just "Chapter 1165"
        // But if there's a real title, we should show it.
        // For now, let's use a cleaner format:
        if (displayName.contains(QString::number(chapter.chapterNumber())) && displayName.startsWith("Chapter", Qt::CaseInsensitive)) {
            item->setText(displayName);
        } else {
            item->setText(QString("Chapter %1: %2").arg(chapter.chapterNumber()).arg(displayName));
        }
        m_chapterListWidget->addItem(item);
    }
}

void MangaDetailsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}
