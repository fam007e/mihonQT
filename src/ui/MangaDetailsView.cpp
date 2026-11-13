#include "MangaDetailsView.h"
#include "ThemeManager.h"
#include "../util/CoverCache.h"
#include "database/DatabaseManager.h"
#include "database/MangaRepository.h"
#include "database/ChapterRepository.h"
#include "database/CategoryRepository.h"
#include "model/ChapterSort.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include "WebViewDialog.h"

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

static QString statusText(int status)
{
    switch (status) {
    case MangaStatus::ONGOING:   return "Ongoing";
    case MangaStatus::COMPLETED: return "Completed";
    case MangaStatus::LICENSED:  return "Licensed";
    case MangaStatus::PUBLISHING_FINISHED: return "Publishing finished";
    case MangaStatus::CANCELLED: return "Cancelled";
    case MangaStatus::ON_HIATUS: return "On hiatus";
    default: return "Unknown";
    }
}

static QString relativeDate(long epochMs)
{
    if (epochMs <= 0) return QString();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(epochMs);
    qint64 secs = dt.secsTo(QDateTime::currentDateTime());
    if (secs < 0) return dt.date().toString("MMM d, yyyy"); // future release
    if (secs < 60) return "now";
    if (secs < 3600) return QStringLiteral("%1m").arg(secs / 60);
    if (secs < 86400) return QStringLiteral("%1h").arg(secs / 3600);
    if (secs < 86400 * 30) return QStringLiteral("%1d").arg(secs / 86400);
    return dt.date().toString("MMM d, yyyy");
}

// One chapter row, matching Mihon's MangaChapterListItem:
// [unread dot] Title            [download icon]
//              date • scanlator
class ChapterRow : public QWidget
{
public:
    ChapterRow(const Chapter& chapter, QWidget *parent = nullptr)
        : QWidget(parent), m_chapter(chapter)
    {
        QVBoxLayout *col = new QVBoxLayout(this);
        col->setContentsMargins(20, 10, 20, 10);
        col->setSpacing(5);

        QHBoxLayout *topRow = new QHBoxLayout();
        topRow->setSpacing(6);
        m_unreadDot = new QLabel(this);
        m_unreadDot->setFixedSize(8, 8);
        m_titleLabel = new QLabel(this);
        m_titleLabel->setObjectName("chapterTitle");
        topRow->addWidget(m_unreadDot);
        topRow->addWidget(m_titleLabel, 1);
        col->addLayout(topRow);

        QHBoxLayout *subRow = new QHBoxLayout();
        subRow->setSpacing(0);
        m_subLabel = new QLabel(this);
        m_subLabel->setObjectName("chapterSub");
        subRow->addWidget(m_subLabel);
        subRow->addStretch();
        col->addLayout(subRow);

        QString title = chapter.name();
        if (title.endsWith(".cbz", Qt::CaseInsensitive) || title.endsWith(".zip", Qt::CaseInsensitive))
            title = QFileInfo(title).completeBaseName();
        m_titleLabel->setText(title);

        QStringList sub;
        QString date = relativeDate(chapter.dateUpload() > 0 ? chapter.dateUpload() : chapter.dateFetch());
        if (!date.isEmpty()) sub << date;
        if (!chapter.scanlator().isEmpty()) sub << chapter.scanlator();
        if (chapter.lastPageRead() > 0 && !chapter.isRead())
            sub << QStringLiteral("Page %1").arg(chapter.lastPageRead() + 1);
        m_subLabel->setText(sub.join("  •  "));

        setCursor(Qt::PointingHandCursor);
        restyle();
    }

    void restyle()
    {
        const M3Colors& c = ThemeManager::instance().colors();
        const bool read = m_chapter.isRead();
        m_titleLabel->setStyleSheet(QStringLiteral(
            "font-size: 14px; background: transparent; color: %1;")
            .arg(read ? ThemeManager::rgba(c.onSurface, 0.38f) : c.onSurface.name()));
        m_subLabel->setStyleSheet(QStringLiteral(
            "font-size: 12px; background: transparent; color: %1;")
            .arg(read ? ThemeManager::rgba(c.onSurface, 0.38f)
                      : ThemeManager::rgba(c.onSurfaceVariant, 0.76f)));
        m_unreadDot->setStyleSheet(read
            ? "background: transparent;"
            : QStringLiteral("background: %1; border-radius: 4px;").arg(c.primary.name()));
    }

    const Chapter& chapter() const { return m_chapter; }
    std::function<void()> onClick; // invoked on left click (avoids moc in .cpp)

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClick) onClick();
        QWidget::mousePressEvent(e);
    }

private:
    Chapter m_chapter;
    QLabel *m_unreadDot;
    QLabel *m_titleLabel;
    QLabel *m_subLabel;
};

// ------------------------------------------------------------------
// MangaDetailsView
// ------------------------------------------------------------------

MangaDetailsView::MangaDetailsView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
    connect(&CoverCache::instance(), &CoverCache::loaded, this, [this](const QString& url, const QPixmap& pm) {
        if (url == m_manga.thumbnailUrl() && m_coverLabel) {
            m_coverLabel->setPixmap(pm.scaled(120, 180, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    });
}

void MangaDetailsView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Top bar: back arrow + title + webview icon (Mihon MangaToolbar) ---
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(56);
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(8, 0, 12, 0);

    QPushButton *backBtn = new QPushButton(topBar);
    backBtn->setText("←");
    backBtn->setFlat(true);
    backBtn->setProperty("flatIcon", true);
    backBtn->setFixedSize(40, 40);
    connect(backBtn, &QPushButton::clicked, this, &MangaDetailsView::backRequested);
    topLayout->addWidget(backBtn);

    m_titleLabel = new QLabel(topBar);
    m_titleLabel->setObjectName("detailsTopTitle");
    topLayout->addWidget(m_titleLabel, 1);

    m_openWebViewButton = new QPushButton(topBar);
    m_openWebViewButton->setIcon(QIcon(":/icons/public.svg"));
    m_openWebViewButton->setToolTip("Open in WebView");
    m_openWebViewButton->setFlat(true);
    m_openWebViewButton->setProperty("flatIcon", true);
    m_openWebViewButton->setFixedSize(40, 40);
    connect(m_openWebViewButton, &QPushButton::clicked, this, [this]() {
        QString fullUrl = m_manga.url();
        if (!fullUrl.isEmpty()) {
            WebViewDialog dialog(fullUrl, this);
            dialog.exec();
        }
    });
    topLayout->addWidget(m_openWebViewButton);
    mainLayout->addWidget(topBar);

    // --- One scrollable column: info header + action chips + chapters ---
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *content = new QWidget();
    content->setStyleSheet("background-color: transparent;");
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);

    // Info header: cover | text column
    QHBoxLayout *infoLayout = new QHBoxLayout();
    infoLayout->setContentsMargins(20, 4, 20, 8);
    infoLayout->setSpacing(16);

    m_coverLabel = new QLabel(content);
    m_coverLabel->setFixedSize(120, 180);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(m_coverLabel);

    QVBoxLayout *textCol = new QVBoxLayout();
    textCol->setSpacing(4);
    QLabel *bigTitle = new QLabel(content);
    bigTitle->setObjectName("detailsBigTitle");
    m_authorLabel = new QLabel(content);
    m_statusLabel = new QLabel(content);
    m_chapterCountLabel = new QLabel(content);
    m_descriptionLabel = new QLabel(content);
    m_descriptionLabel->setWordWrap(true);
    textCol->addWidget(bigTitle);
    textCol->addWidget(m_authorLabel);
    textCol->addWidget(m_statusLabel);
    textCol->addWidget(m_chapterCountLabel);
    textCol->addWidget(m_descriptionLabel, 1);
    infoLayout->addLayout(textCol, 1);
    contentLayout->addLayout(infoLayout);
    m_titleLabel = bigTitle; // big title in header column

    // Action chips row (Mihon MangaInfoHeader action row)
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setContentsMargins(20, 4, 20, 8);
    actionsLayout->setSpacing(12);
    actionsLayout->addStretch();

    m_libraryButton = new QPushButton(content);
    m_libraryButton->setToolTip("Add to/Remove from library");
    m_libraryButton->setFixedSize(56, 44);
    connect(m_libraryButton, &QPushButton::clicked, this, [this]() {
        MangaRepository& repo = DatabaseManager::instance().mangaRepository();
        m_manga.setFavorite(!m_manga.favorite());
        if (repo.updateManga(m_manga)) {
            updateLibraryButton();
            emit libraryStatusChanged();
        } else {
            QMessageBox::warning(this, "Error", "Failed to update library status.");
            m_manga.setFavorite(!m_manga.favorite());
        }
    });
    actionsLayout->addWidget(m_libraryButton);

    m_categoriesButton = new QPushButton(content);
    m_categoriesButton->setText("Categories");
    connect(m_categoriesButton, &QPushButton::clicked, this, [this]() {
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
            if (currentCatIds.contains(cat.id())) cb->setChecked(true);
            layout->addWidget(cb);
            checkBoxes.append(cb);
        }

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttonBox);

        if (dialog.exec() == QDialog::Accepted) {
            QList<long> newCatIds;
            for (QCheckBox *cb : checkBoxes)
                if (cb->isChecked()) newCatIds.append(cb->property("categoryId").toLongLong());
            catRepo.setCategoriesForManga(m_manga.id(), newCatIds);
            emit libraryStatusChanged();
        }
    });
    actionsLayout->addWidget(m_categoriesButton);
    actionsLayout->addStretch();
    contentLayout->addLayout(actionsLayout);

    // --- Chapter section header: "N chapters" + sort/filter buttons ---
    QWidget *chapterHeader = new QWidget(content);
    QHBoxLayout *chLayout = new QHBoxLayout(chapterHeader);
    chLayout->setContentsMargins(20, 8, 12, 4);
    m_chaptersHeaderLabel = new QLabel(chapterHeader);
    chLayout->addWidget(m_chaptersHeaderLabel);
    chLayout->addStretch();

    m_sortButton = new QPushButton(chapterHeader);
    m_sortButton->setFlat(true);
    m_sortButton->setProperty("flatIcon", true);
    connect(m_sortButton, &QPushButton::clicked, this, [this]() {
        // Toggle direction (matching Android Mihon tap behavior)
        long flags = m_manga.chapterFlags();
        long dir = flags & Manga::CHAPTER_SORT_DIR_MASK;
        long newDir = (dir == Manga::CHAPTER_SORT_DESC) ? Manga::CHAPTER_SORT_ASC : Manga::CHAPTER_SORT_DESC;
        flags = (flags & ~Manga::CHAPTER_SORT_DIR_MASK) | newDir;
        m_manga.setChapterFlags(flags);
        DatabaseManager::instance().mangaRepository().updateManga(m_manga);
        updateSortButton();
        refreshChapters();
    });
    chLayout->addWidget(m_sortButton);

    m_filterButton = new QPushButton(chapterHeader);
    m_filterButton->setIcon(QIcon(":/icons/filter.svg"));
    m_filterButton->setFlat(true);
    m_filterButton->setProperty("flatIcon", true);
    m_filterButton->setFixedSize(40, 40);
    chLayout->addWidget(m_filterButton);
    contentLayout->addWidget(chapterHeader);

    // Chapter rows
    m_chapterListContainer = new QWidget(content);
    m_chapterListContainer->setStyleSheet("background-color: transparent;");
    m_chapterListLayout = new QVBoxLayout(m_chapterListContainer);
    m_chapterListLayout->setContentsMargins(0, 0, 0, 24);
    m_chapterListLayout->setSpacing(0);
    contentLayout->addWidget(m_chapterListContainer);

    m_scrollArea->setWidget(content);
    mainLayout->addWidget(m_scrollArea);

    restyle();
}

void MangaDetailsView::restyle()
{
    const M3Colors& c = ThemeManager::instance().colors();

    if (auto *top = findChild<QLabel*>("detailsTopTitle")) {
        // (top bar title was repurposed; keep harmless)
        top->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600; color: %1; background: transparent;")
                               .arg(c.onBackground.name()));
    }
    m_titleLabel->setStyleSheet(QStringLiteral(
        "font-size: 20px; font-weight: 600; color: %1; background: transparent;")
        .arg(c.onBackground.name()));
    m_authorLabel->setStyleSheet(QStringLiteral(
        "font-size: 12px; color: %1; background: transparent;")
        .arg(ThemeManager::rgba(c.onSurfaceVariant, 0.86f)));
    m_statusLabel->setStyleSheet(QStringLiteral(
        "font-size: 12px; color: %1; background: transparent;")
        .arg(ThemeManager::rgba(c.onSurfaceVariant, 0.86f)));
    m_chapterCountLabel->setStyleSheet(QStringLiteral(
        "font-size: 12px; color: %1; background: transparent;")
        .arg(ThemeManager::rgba(c.onSurfaceVariant, 0.86f)));
    m_descriptionLabel->setStyleSheet(QStringLiteral(
        "font-size: 13px; color: %1; background: transparent;")
        .arg(ThemeManager::rgba(c.onSurfaceVariant, 0.86f)));
    m_chaptersHeaderLabel->setStyleSheet(QStringLiteral(
        "font-size: 14px; font-weight: 600; color: %1; background: transparent;")
        .arg(c.onBackground.name()));
    m_libraryButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: 1px solid %1; border-radius: 10px; background: transparent;"
        "  color: %2; font-size: 16px; }"
        "QPushButton:hover { background: %3; }")
        .arg(c.outlineVariant.name(), c.onSurface.name(),
             ThemeManager::rgba(c.onSurface, 0.08f)));
    m_categoriesButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: 1px solid %1; border-radius: 22px; background: transparent;"
        "  color: %2; padding: 10px 18px; font-weight: 500; }"
        "QPushButton:hover { background: %3; }")
        .arg(c.outlineVariant.name(), c.onSurface.name(),
             ThemeManager::rgba(c.onSurface, 0.08f)));

    // Restyle all chapter rows (ChapterRow has no Q_OBJECT; walk the layout)
    for (int i = 0; i < m_chapterListLayout->count(); ++i) {
        QLayoutItem *item = m_chapterListLayout->itemAt(i);
        if (auto *row = dynamic_cast<ChapterRow*>(item->widget()))
            row->restyle();
    }
}

void MangaDetailsView::setManga(const Manga& manga)
{
    m_manga = manga;
    updateHeader();
    updateLibraryButton();
    updateSortButton();
    refreshChapters();
}

void MangaDetailsView::updateHeader()
{
    m_titleLabel->setText(m_manga.title());
    m_titleLabel->setWordWrap(true);

    QStringList authorLine;
    if (!m_manga.author().isEmpty()) authorLine << m_manga.author();
    if (!m_manga.artist().isEmpty() && m_manga.artist() != m_manga.author())
        authorLine << m_manga.artist();
    m_authorLabel->setText(authorLine.join(" · "));

    m_statusLabel->setText(statusText(m_manga.status()));

    // Cover (cached async load)
    m_coverLabel->setText("…");
    QString thumb = m_manga.thumbnailUrl();
    if (thumb.isEmpty() && m_manga.source() == 0) {
        QDir dir(m_manga.url());
        QStringList filters = {"cover.*", "folder.*", "poster.*", "*.jpg", "*.png", "*.jpeg"};
        QFileInfoList list = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        if (!list.isEmpty()) thumb = list.first().absoluteFilePath();
    }
    if (!thumb.isEmpty()) {
        QPixmap pm = thumb.startsWith("http")
            ? CoverCache::instance().load(thumb)
            : QPixmap(thumb);
        if (!pm.isNull())
            m_coverLabel->setPixmap(pm.scaled(120, 180, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void MangaDetailsView::updateLibraryButton()
{
    m_libraryButton->setText(m_manga.favorite()
        ? QStringLiteral("✓")          // in library
        : QStringLiteral("＋"));        // not in library
    m_libraryButton->setToolTip(m_manga.favorite() ? "Remove from Library" : "Add to Library");
}

void MangaDetailsView::updateSortButton()
{
    QString dirStr = m_manga.sortDescending() ? "↓" : "↑";
    m_sortButton->setText(dirStr);
    m_sortButton->setToolTip("Toggle chapter sort direction");
}

void MangaDetailsView::refreshChapters()
{
    // Clear rows
    QLayoutItem *item;
    while ((item = m_chapterListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    ChapterRepository& repo = DatabaseManager::instance().chapterRepository();
    m_chapters = repo.getChaptersByMangaId(m_manga.id());

    auto comparator = getChapterComparator(m_manga, m_manga.sortDescending());
    std::sort(m_chapters.begin(), m_chapters.end(), comparator);

    int read = 0;
    for (const Chapter& chapter : m_chapters) {
        if (chapter.isRead()) ++read;
        ChapterRow *row = new ChapterRow(chapter, m_chapterListContainer);
        row->onClick = [this, row]() {
            emit chapterSelected(m_manga, row->chapter());
        };
        m_chapterListLayout->addWidget(row);
    }

    m_chaptersHeaderLabel->setText(
        QStringLiteral("%1 chapters  ·  %2 read").arg(m_chapters.size()).arg(read));
}

void MangaDetailsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}
