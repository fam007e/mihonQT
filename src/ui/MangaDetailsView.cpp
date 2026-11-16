#include "MangaDetailsView.h"
#include "ThemeManager.h"
#include "../util/CoverCache.h"
#include "database/DatabaseManager.h"
#include "database/MangaRepository.h"
#include "database/ChapterRepository.h"
#include "database/CategoryRepository.h"
#include "model/ChapterSort.h"
#include <QUrl>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QTextDocumentFragment>
#include <QRegularExpression>
#include "WebViewDialog.h"
#include "TrackingWidget.h"
#include "../tracking/TrackerManager.h"

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

static QString cleanSynopsis(const QString& raw)
{
    if (raw.isEmpty()) return QString();
    QString text = raw;

    // Convert <br>, <br/>, <p> to newlines
    text.replace(QRegularExpression("<br\\s*/?>", QRegularExpression::CaseInsensitiveOption), "\n");
    text.replace(QRegularExpression("</p>", QRegularExpression::CaseInsensitiveOption), "\n\n");
    text.replace(QRegularExpression("<p[^>]*>", QRegularExpression::CaseInsensitiveOption), "");

    // Strip remaining HTML tags
    text.replace(QRegularExpression("<[^>]+>"), "");

    // Unescape common HTML entities
    text.replace("&quot;", "\"");
    text.replace("&#039;", "'");
    text.replace("&apos;", "'");
    text.replace("&rsquo;", "’");
    text.replace("&lsquo;", "‘");
    text.replace("&ldquo;", "“");
    text.replace("&rdquo;", "”");
    text.replace("&amp;", "&");
    text.replace("&lt;", "<");
    text.replace("&gt;", ">");
    text.replace("&nbsp;", " ");
    text.replace("&hellip;", "…");
    text.replace("&mdash;", "—");
    text.replace("&ndash;", "–");

    // Also use QTextDocumentFragment for any remaining numerical entities
    if (text.contains('&')) {
        QTextDocumentFragment frag = QTextDocumentFragment::fromHtml(text);
        QString plain = frag.toPlainText();
        if (!plain.isEmpty()) text = plain;
    }

    // Normalize multiple consecutive blank lines
    text.replace(QRegularExpression("\n{3,}"), "\n\n");
    return text.trimmed();
}

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
// [checkbox] [unread dot] Title            [bookmark icon]
//                         date • scanlator
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
        topRow->setSpacing(8);

        m_checkBox = new QCheckBox(this);
        m_checkBox->setVisible(false);
        m_checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_checkBox->setFocusPolicy(Qt::NoFocus);
        connect(m_checkBox, &QCheckBox::toggled, this, [this](bool checked) {
            update();
            if (onSelectionToggled) onSelectionToggled(checked);
        });
        topRow->addWidget(m_checkBox);

        m_unreadDot = new QLabel(this);
        m_unreadDot->setFixedSize(8, 8);
        m_titleLabel = new QLabel(this);
        m_titleLabel->setObjectName("chapterTitle");
        topRow->addWidget(m_unreadDot);
        topRow->addWidget(m_titleLabel, 1);

        m_bookmarkBadge = new QLabel(this);
        m_bookmarkBadge->setText("★");
        m_bookmarkBadge->setStyleSheet("color: #EBCB8B; font-size: 14px; background: transparent;");
        m_bookmarkBadge->setVisible(chapter.bookmark());
        topRow->addWidget(m_bookmarkBadge);

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

        // Make labels pass mouse events through so clicks and right-clicks hit ChapterRow directly
        m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_subLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_unreadDot->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_bookmarkBadge->setAttribute(Qt::WA_TransparentForMouseEvents);

        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus); // keyboard: Up/Down moves, Enter opens
        restyle();
    }

    void setSelectionMode(bool mode)
    {
        m_selectionMode = mode;
        m_checkBox->setVisible(mode);
        if (!mode) {
            m_checkBox->setChecked(false);
        }
        restyle();
        update();
    }

    bool isSelected() const { return m_checkBox && m_checkBox->isChecked(); }
    void setSelected(bool sel)
    {
        if (m_checkBox && m_checkBox->isChecked() != sel) {
            m_checkBox->setChecked(sel);
            update();
        }
    }

    void restyle()
    {
        const M3Colors& c = ThemeManager::instance().colors();
        const bool isRead = m_chapter.isRead();
        // Mihon Android style: read chapters are dimmed (38% opacity)
        m_titleLabel->setStyleSheet(QStringLiteral(
            "font-size: 14px; background: transparent; color: %1; %2")
            .arg(isRead ? ThemeManager::rgba(c.onSurface, 0.38f) : c.onSurface.name(),
                 isRead ? "" : "font-weight: 500;"));
        m_subLabel->setStyleSheet(QStringLiteral(
            "font-size: 12px; background: transparent; color: %1;")
            .arg(isRead ? ThemeManager::rgba(c.onSurface, 0.38f)
                      : c.onSurfaceVariant.name()));
        m_unreadDot->setVisible(!isRead && !m_selectionMode);
        m_unreadDot->setStyleSheet(QStringLiteral("background: %1; border-radius: 4px;").arg(c.primary.name()));
        if (m_bookmarkBadge) {
            m_bookmarkBadge->setVisible(m_chapter.bookmark());
        }
    }

    const Chapter& chapter() const { return m_chapter; }
    std::function<void()> onClick; // invoked on left click (avoids moc in .cpp)
    std::function<void(ChapterRow*, bool)> onRowClickedInSelection;
    std::function<void(bool)> onSelectionToggled;
    std::function<void(const Chapter&)> onMarkRead;
    std::function<void(const Chapter&)> onMarkUnread;
    std::function<void(const Chapter&)> onMarkPreviousRead;
    std::function<void(const Chapter&)> onToggleBookmark;
    std::function<void()> onStartSelection;

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) {
            bool shift = (e->modifiers() & Qt::ShiftModifier);
            if (m_selectionMode) {
                if (onRowClickedInSelection) {
                    onRowClickedInSelection(this, shift);
                } else {
                    setSelected(!isSelected());
                }
            } else if (onClick) {
                onClick();
            }
            e->accept();
            return;
        } else if (e->button() == Qt::RightButton) {
            e->accept();
            return;
        }
        QWidget::mousePressEvent(e);
    }

    void contextMenuEvent(QContextMenuEvent *e) override
    {
        e->accept();
        QMenu menu(this);
        if (m_chapter.isRead()) {
            menu.addAction("Mark as unread", [this]() {
                if (onMarkUnread) onMarkUnread(m_chapter);
            });
        } else {
            menu.addAction("Mark as read", [this]() {
                if (onMarkRead) onMarkRead(m_chapter);
            });
        }

        menu.addAction("Mark previous chapters as read", [this]() {
            if (onMarkPreviousRead) onMarkPreviousRead(m_chapter);
        });

        if (m_chapter.bookmark()) {
            menu.addAction("Remove bookmark", [this]() {
                if (onToggleBookmark) onToggleBookmark(m_chapter);
            });
        } else {
            menu.addAction("Bookmark", [this]() {
                if (onToggleBookmark) onToggleBookmark(m_chapter);
            });
        }

        menu.addSeparator();
        menu.addAction("Select multiple chapters", [this]() {
            if (onStartSelection) onStartSelection();
        });

        menu.exec(e->globalPos());
    }

    void keyPressEvent(QKeyEvent *e) override
    {
        switch (e->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            if (m_selectionMode) {
                m_checkBox->setChecked(!m_checkBox->isChecked());
            } else if (onClick) {
                onClick();
            }
            e->accept();
            return;
        default:
            QWidget::keyPressEvent(e);
        }
    }

    void paintEvent(QPaintEvent *e) override
    {
        QWidget::paintEvent(e);
        if (m_selectionMode && isSelected()) {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            p.fillRect(rect(), ThemeManager::rgba(ThemeManager::instance().colors().primary, 0.18f));
            QPen pen(ThemeManager::instance().colors().primary);
            pen.setWidthF(1.5);
            p.setPen(pen);
            p.drawRoundedRect(rect().adjusted(8, 2, -8, -2), 8, 8);
        } else if (hasFocus()) {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            QPen pen(ThemeManager::instance().colors().primary);
            pen.setWidthF(1.5);
            p.setPen(pen);
            p.drawRoundedRect(rect().adjusted(8, 2, -8, -2), 8, 8);
        }
    }

private:
    Chapter m_chapter;
    bool m_selectionMode = false;
    QCheckBox *m_checkBox = nullptr;
    QLabel *m_unreadDot = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subLabel = nullptr;
    QLabel *m_bookmarkBadge = nullptr;
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
    textCol->setSpacing(6);
    QLabel *bigTitle = new QLabel(content);
    bigTitle->setObjectName("detailsBigTitle");
    m_authorLabel = new QLabel(content);
    m_statusLabel = new QLabel(content);
    m_chapterCountLabel = new QLabel(content);
    textCol->addWidget(bigTitle);
    textCol->addWidget(m_authorLabel);
    textCol->addWidget(m_statusLabel);
    textCol->addWidget(m_chapterCountLabel);
    textCol->addStretch();
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

    m_trackingButton = new QPushButton(content);
    m_trackingButton->setText("Tracking");
    connect(m_trackingButton, &QPushButton::clicked, this, [this]() {
        QDialog dialog(this);
        dialog.setWindowTitle(QString("Tracking - %1").arg(m_manga.title()));
        dialog.resize(520, 560);
        QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);
        dlgLayout->setContentsMargins(0, 0, 0, 0);

        TrackingWidget *widget = new TrackingWidget(m_manga.id(), m_manga.title(), &dialog);
        connect(widget, &TrackingWidget::closeRequested, &dialog, &QDialog::accept);
        dlgLayout->addWidget(widget);

        dialog.exec();
    });
    actionsLayout->addWidget(m_trackingButton);

    actionsLayout->addStretch();
    contentLayout->addLayout(actionsLayout);

    // Genre tags container (horizontal scroll)
    QScrollArea *genreScroll = new QScrollArea(content);
    genreScroll->setWidgetResizable(true);
    genreScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    genreScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    genreScroll->setFrameShape(QFrame::NoFrame);
    genreScroll->setStyleSheet("background-color: transparent;");
    genreScroll->setFixedHeight(38);

    m_genreContainer = new QWidget(genreScroll);
    m_genreContainer->setStyleSheet("background-color: transparent;");
    m_genreLayout = new QHBoxLayout(m_genreContainer);
    m_genreLayout->setContentsMargins(20, 2, 20, 2);
    m_genreLayout->setSpacing(8);
    m_genreLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    genreScroll->setWidget(m_genreContainer);
    contentLayout->addWidget(genreScroll);

    // Description / Synopsis container
    QWidget *descContainer = new QWidget(content);
    descContainer->setStyleSheet("background-color: transparent;");
    QVBoxLayout *descLayout = new QVBoxLayout(descContainer);
    descLayout->setContentsMargins(20, 4, 20, 6);
    descLayout->setSpacing(4);

    m_descriptionLabel = new QLabel(descContainer);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    descLayout->addWidget(m_descriptionLabel);

    m_moreButton = new QPushButton(descContainer);
    m_moreButton->setFlat(true);
    m_moreButton->setCursor(Qt::PointingHandCursor);
    connect(m_moreButton, &QPushButton::clicked, this, [this]() {
        m_descriptionExpanded = !m_descriptionExpanded;
        updateDescriptionText();
    });
    descLayout->addWidget(m_moreButton);
    contentLayout->addWidget(descContainer);

    // --- Chapter section header: "N chapters" + sort/filter buttons ---
    QWidget *chapterHeader = new QWidget(content);
    QHBoxLayout *chLayout = new QHBoxLayout(chapterHeader);
    chLayout->setContentsMargins(20, 8, 12, 4);
    m_chaptersHeaderLabel = new QLabel(chapterHeader);
    chLayout->addWidget(m_chaptersHeaderLabel);
    chLayout->addStretch();

    m_selectButton = new QPushButton(chapterHeader);
    m_selectButton->setText("Select");
    m_selectButton->setFlat(true);
    m_selectButton->setCursor(Qt::PointingHandCursor);
    connect(m_selectButton, &QPushButton::clicked, this, [this]() {
        setSelectionMode(!m_selectionMode);
    });
    chLayout->addWidget(m_selectButton);

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
    // Mihon-style chapter filter menu
    QMenu *filterMenu = new QMenu(m_filterButton);
    QAction *allAct = filterMenu->addAction("All chapters");
    QAction *unreadAct = filterMenu->addAction("Unread");
    QAction *dlAct = filterMenu->addAction("Downloaded");
    QAction *bmAct = filterMenu->addAction("Bookmarked");
    for (QAction *a : {allAct, unreadAct, dlAct, bmAct}) a->setCheckable(true);
    allAct->setChecked(true);
    auto setFilter = [this, allAct, unreadAct, dlAct, bmAct](ChapterFilter f) {
        m_chapterFilter = f;
        for (QAction *a : {allAct, unreadAct, dlAct, bmAct})
            a->setChecked(a == allAct ? (f == All) : a == unreadAct ? (f == UnreadOnly)
                                    : a == dlAct ? (f == DownloadedOnly) : (f == BookmarkedOnly));
        refreshChapters();
    };
    connect(allAct, &QAction::triggered, this, [setFilter]() { setFilter(All); });
    connect(unreadAct, &QAction::triggered, this, [setFilter]() { setFilter(UnreadOnly); });
    connect(dlAct, &QAction::triggered, this, [setFilter]() { setFilter(DownloadedOnly); });
    connect(bmAct, &QAction::triggered, this, [setFilter]() { setFilter(BookmarkedOnly); });
    connect(m_filterButton, &QPushButton::clicked, this, [this, filterMenu]() {
        filterMenu->popup(m_filterButton->mapToGlobal(QPoint(0, m_filterButton->height())));
    });
    chLayout->addWidget(m_filterButton);
    contentLayout->addWidget(chapterHeader);

    // Multi-chapter selection action bar
    m_selectionBar = new QWidget(content);
    m_selectionBar->setVisible(false);
    QHBoxLayout *selLayout = new QHBoxLayout(m_selectionBar);
    selLayout->setContentsMargins(20, 4, 12, 4);
    selLayout->setSpacing(8);

    m_selectionCountLabel = new QLabel("0 selected", m_selectionBar);
    m_selectionCountLabel->setStyleSheet("font-weight: bold;");
    selLayout->addWidget(m_selectionCountLabel);
    selLayout->addStretch();

    m_selectAllBtn = new QPushButton("Select All", m_selectionBar);
    connect(m_selectAllBtn, &QPushButton::clicked, this, [this]() {
        bool allSelected = true;
        for (ChapterRow *row : chapterRows()) {
            if (!row->isSelected()) { allSelected = false; break; }
        }
        for (ChapterRow *row : chapterRows()) {
            row->setSelected(!allSelected);
        }
        updateSelectionCount();
    });
    selLayout->addWidget(m_selectAllBtn);

    m_markReadBtn = new QPushButton("Mark as read", m_selectionBar);
    connect(m_markReadBtn, &QPushButton::clicked, this, [this]() {
        markSelectedAsRead(true);
    });
    selLayout->addWidget(m_markReadBtn);

    m_markUnreadBtn = new QPushButton("Mark as unread", m_selectionBar);
    connect(m_markUnreadBtn, &QPushButton::clicked, this, [this]() {
        markSelectedAsRead(false);
    });
    selLayout->addWidget(m_markUnreadBtn);

    m_bookmarkBtn = new QPushButton("Bookmark", m_selectionBar);
    connect(m_bookmarkBtn, &QPushButton::clicked, this, [this]() {
        bookmarkSelected();
    });
    selLayout->addWidget(m_bookmarkBtn);

    m_cancelSelectBtn = new QPushButton("Done", m_selectionBar);
    connect(m_cancelSelectBtn, &QPushButton::clicked, this, [this]() {
        setSelectionMode(false);
    });
    selLayout->addWidget(m_cancelSelectBtn);

    contentLayout->addWidget(m_selectionBar);

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
        .arg(c.onSurfaceVariant.name()));
    m_statusLabel->setStyleSheet(QStringLiteral(
        "font-size: 12px; color: %1; background: transparent;")
        .arg(c.onSurfaceVariant.name()));
    m_chapterCountLabel->setStyleSheet(QStringLiteral(
        "font-size: 12px; color: %1; background: transparent;")
        .arg(c.onSurfaceVariant.name()));
    m_descriptionLabel->setStyleSheet(QStringLiteral(
        "font-size: 13px; line-height: 1.4; color: %1; background: transparent;")
        .arg(c.onSurfaceVariant.name()));
    m_moreButton->setStyleSheet(QStringLiteral(
        "QPushButton { font-size: 12px; font-weight: 600; color: %1; background: transparent; text-align: left; padding: 2px 0; border: none; }"
        "QPushButton:hover { text-decoration: underline; }")
        .arg(c.primary.name()));
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
    m_trackingButton->setStyleSheet(QStringLiteral(
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
        const QUrl mangaUrl(m_manga.url());
        const QString referer = (mangaUrl.isValid() && (mangaUrl.scheme() == "http" || mangaUrl.scheme() == "https"))
            ? mangaUrl.scheme() + "://" + mangaUrl.host() + "/" : QString();
        QPixmap pm = thumb.startsWith("http")
            ? CoverCache::instance().load(thumb, referer)
            : QPixmap(thumb);
        if (!pm.isNull())
            m_coverLabel->setPixmap(pm.scaled(120, 180, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    // Genre tags
    while (QLayoutItem *item = m_genreLayout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    const M3Colors& c = ThemeManager::instance().colors();
    QStringList genres = m_manga.genre().split(',', Qt::SkipEmptyParts);
    if (genres.isEmpty()) {
        m_genreContainer->parentWidget()->hide();
    } else {
        m_genreContainer->parentWidget()->show();
        for (const QString& g : genres) {
            QString trimmed = g.trimmed();
            if (trimmed.isEmpty()) continue;
            QLabel *chip = new QLabel(trimmed, m_genreContainer);
            chip->setStyleSheet(QStringLiteral(
                "QLabel { background: %1; color: %2; border-radius: 12px; padding: 4px 10px; font-size: 11px; font-weight: 500; border: 1px solid %3; }"
            ).arg(ThemeManager::rgba(c.primary, 0.12f), c.onSurface.name(), ThemeManager::rgba(c.outlineVariant, 0.4f)));
            m_genreLayout->addWidget(chip);
        }
        m_genreLayout->addStretch();
    }

    // Synopsis
    m_descriptionExpanded = false;
    updateDescriptionText();
}

void MangaDetailsView::updateDescriptionText()
{
    QString desc = cleanSynopsis(m_manga.description());
    if (desc.isEmpty()) {
        m_descriptionLabel->hide();
        m_moreButton->hide();
        return;
    }
    m_descriptionLabel->show();

    if (desc.length() > 250 && !m_descriptionExpanded) {
        m_descriptionLabel->setText(desc.left(250) + "…");
        m_moreButton->setText("More ▼");
        m_moreButton->show();
    } else {
        m_descriptionLabel->setText(desc);
        if (desc.length() > 250) {
            m_moreButton->setText("Less ▲");
            m_moreButton->show();
        } else {
            m_moreButton->hide();
        }
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

    int readCount = 0, shown = 0;
    for (const Chapter& chapter : m_chapters) {
        if (chapter.isRead()) ++readCount;
        // Apply the filter menu selection
        switch (m_chapterFilter) {
        case UnreadOnly:      if (chapter.isRead()) continue; break;
        case DownloadedOnly:  if (chapter.url().startsWith("http")) continue; break;
        case BookmarkedOnly:  if (!chapter.bookmark()) continue; break;
        case All:             break;
        }
        ++shown;
        ChapterRow *row = new ChapterRow(chapter, m_chapterListContainer);
        if (m_selectionMode) {
            row->setSelectionMode(true);
        }
        row->onClick = [this, row]() {
            if (m_selectionMode) {
                row->setSelected(!row->isSelected());
                updateSelectionCount();
            } else {
                emit chapterSelected(m_manga, row->chapter());
            }
        };
        row->onRowClickedInSelection = [this](ChapterRow *targetRow, bool shift) {
            const QList<ChapterRow*> allRows = chapterRows();
            int currentIdx = allRows.indexOf(targetRow);
            if (currentIdx < 0) return;

            int anchorIdx = m_lastSelectedRowIndex;
            if (anchorIdx < 0) {
                for (int i = 0; i < allRows.size(); ++i) {
                    if (allRows[i]->isSelected() && allRows[i] != targetRow) {
                        anchorIdx = i;
                        break;
                    }
                }
            }

            if (shift && anchorIdx >= 0 && anchorIdx < allRows.size()) {
                int start = qMin(anchorIdx, currentIdx);
                int end = qMax(anchorIdx, currentIdx);
                for (int i = start; i <= end; ++i) {
                    allRows[i]->setSelected(true);
                }
            } else {
                targetRow->setSelected(!targetRow->isSelected());
                m_lastSelectedRowIndex = currentIdx;
            }
            updateSelectionCount();
        };
        row->onSelectionToggled = [this](bool) {
            updateSelectionCount();
        };
        row->onMarkRead = [this](const Chapter& ch) {
            Chapter copy = ch;
            copy.setIsRead(true);
            copy.setLastPageRead(0);
            DatabaseManager::instance().chapterRepository().updateChapter(copy);
            if (ch.chapterNumber() > 0) {
                TrackerManager::instance().updateProgress(m_manga.id(), static_cast<int>(ch.chapterNumber()));
            }
            refreshChapters();
        };
        row->onMarkUnread = [this](const Chapter& ch) {
            Chapter copy = ch;
            copy.setIsRead(false);
            DatabaseManager::instance().chapterRepository().updateChapter(copy);
            refreshChapters();
        };
        row->onMarkPreviousRead = [this](const Chapter& target) {
            ChapterRepository& repo = DatabaseManager::instance().chapterRepository();
            // Take all chapters in reading order (ascending: Ch 1, 2, ... target)
            QList<Chapter> ascending = m_chapters;
            auto comp = getChapterComparator(m_manga, false); // ascending
            std::sort(ascending.begin(), ascending.end(), comp);

            int targetIdx = -1;
            for (int i = 0; i < ascending.size(); ++i) {
                if (ascending[i].id() == target.id()) {
                    targetIdx = i;
                    break;
                }
            }

            int lastReadCh = 0;
            // Strictly mark chapters PREVIOUS to target as read (i < targetIdx), leaving target unread
            for (int i = 0; i < targetIdx && i < ascending.size(); ++i) {
                Chapter ch = ascending[i];
                ch.setIsRead(true);
                ch.setLastPageRead(0);
                repo.updateChapter(ch);
                if (ch.chapterNumber() > lastReadCh) {
                    lastReadCh = static_cast<int>(ch.chapterNumber());
                }
            }
            if (lastReadCh > 0) {
                TrackerManager::instance().updateProgress(m_manga.id(), lastReadCh);
            }
            refreshChapters();
        };
        row->onToggleBookmark = [this](const Chapter& ch) {
            Chapter copy = ch;
            copy.setBookmark(!copy.bookmark());
            DatabaseManager::instance().chapterRepository().updateChapter(copy);
            refreshChapters();
        };
        row->onStartSelection = [this, row]() {
            setSelectionMode(true);
            row->setSelected(true);
            m_lastSelectedRowIndex = chapterRows().indexOf(row);
            updateSelectionCount();
        };
        m_chapterListLayout->addWidget(row);
    }

    if (m_chapterFilter == All)
        m_chaptersHeaderLabel->setText(
            QStringLiteral("%1 chapters  ·  %2 read").arg(m_chapters.size()).arg(readCount));
    else
        m_chaptersHeaderLabel->setText(
            QStringLiteral("%1 of %2 chapters").arg(shown).arg(m_chapters.size()));
}

void MangaDetailsView::setSelectionMode(bool active)
{
    m_selectionMode = active;
    m_lastSelectedRowIndex = -1;
    m_selectionBar->setVisible(active);
    m_selectButton->setText(active ? "Cancel" : "Select");
    for (ChapterRow *row : chapterRows()) {
        row->setSelectionMode(active);
    }
    updateSelectionCount();
}

void MangaDetailsView::updateSelectionCount()
{
    int count = 0;
    for (ChapterRow *row : chapterRows()) {
        if (row->isSelected()) ++count;
    }
    m_selectionCountLabel->setText(QString("%1 selected").arg(count));
    m_markReadBtn->setEnabled(count > 0);
    m_markUnreadBtn->setEnabled(count > 0);
    m_bookmarkBtn->setEnabled(count > 0);
}

void MangaDetailsView::markSelectedAsRead(bool markAsRead)
{
    ChapterRepository& repo = DatabaseManager::instance().chapterRepository();
    int maxCh = 0;
    for (ChapterRow *row : chapterRows()) {
        if (row->isSelected()) {
            Chapter ch = row->chapter();
            ch.setIsRead(markAsRead);
            if (markAsRead) {
                ch.setLastPageRead(0);
                if (ch.chapterNumber() > maxCh) {
                    maxCh = static_cast<int>(ch.chapterNumber());
                }
            }
            repo.updateChapter(ch);
        }
    }
    if (markAsRead && maxCh > 0) {
        TrackerManager::instance().updateProgress(m_manga.id(), maxCh);
    }
    setSelectionMode(false);
    refreshChapters();
}

void MangaDetailsView::bookmarkSelected()
{
    ChapterRepository& repo = DatabaseManager::instance().chapterRepository();
    for (ChapterRow *row : chapterRows()) {
        if (row->isSelected()) {
            Chapter ch = row->chapter();
            ch.setBookmark(!ch.bookmark());
            repo.updateChapter(ch);
        }
    }
    setSelectionMode(false);
    refreshChapters();
}

void MangaDetailsView::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

QList<ChapterRow *> MangaDetailsView::chapterRows() const
{
    QList<ChapterRow*> rows;
    for (int i = 0; i < m_chapterListLayout->count(); ++i) {
        QLayoutItem *item = m_chapterListLayout->itemAt(i);
        if (auto *row = dynamic_cast<ChapterRow*>(item->widget()))
            rows.append(row);
    }
    return rows;
}

void MangaDetailsView::keyPressEvent(QKeyEvent *event)
{
    if (m_selectionMode && event->key() == Qt::Key_Escape) {
        setSelectionMode(false);
        event->accept();
        return;
    }

    const QList<ChapterRow*> rows = chapterRows();
    QWidget *focus = focusWidget();
    int current = -1;
    for (int i = 0; i < rows.size(); ++i)
        if (rows[i] == focus) { current = i; break; }

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Left:
        if (rows.isEmpty()) break;
        current = (current <= 0) ? rows.size() - 1 : current - 1;
        rows[current]->setFocus();
        m_scrollArea->ensureWidgetVisible(rows[current]);
        event->accept();
        return;
    case Qt::Key_Down:
    case Qt::Key_Right:
        if (rows.isEmpty()) break;
        current = (current < 0 || current >= rows.size() - 1) ? 0 : current + 1;
        rows[current]->setFocus();
        m_scrollArea->ensureWidgetVisible(rows[current]);
        event->accept();
        return;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}

void MangaDetailsView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // Focus the first chapter row so Up/Down work immediately
    const QList<ChapterRow*> rows = chapterRows();
    if (!rows.isEmpty()) rows.first()->setFocus();
}
