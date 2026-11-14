#include "LibraryView.h"
#include "MangaCoverCard.h"
#include "ThemeManager.h"
#include "../database/DatabaseManager.h"
#include "../database/MangaRepository.h"
#include "../database/ChapterRepository.h"
#include "../database/CategoryRepository.h"
#include "../config/PreferenceManager.h"
#include "../model/Chapter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>

LibraryView::LibraryView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
}

void LibraryView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Toolbar (Mihon: title + search/filter/more icons) ---
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(60);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);

    QLabel *titleLabel = new QLabel("Library", headerWidget);
    titleLabel->setObjectName("libraryTitle");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    auto createHeaderBtn = [headerWidget](const QString& iconName, const QString& tip) {
        QPushButton* btn = new QPushButton(headerWidget);
        btn->setIcon(QIcon(QString(":/icons/%1.svg").arg(iconName)));
        btn->setIconSize(QSize(22, 22));
        btn->setFixedSize(40, 40);
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(tip);
        btn->setProperty("flatIcon", true);
        return btn;
    };

    m_searchBtn = createHeaderBtn("search", "Search library");
    m_filterBtn = createHeaderBtn("filter", "Filter");
    m_moreBtn = createHeaderBtn("more", "More");

    headerLayout->addWidget(m_searchBtn);
    headerLayout->addWidget(m_filterBtn);
    headerLayout->addWidget(m_moreBtn);

    mainLayout->addWidget(headerWidget);

    // --- Search bar (hidden until the search icon toggles it) ---
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search library...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(42);
    m_searchEdit->hide();
    mainLayout->addWidget(m_searchEdit);

    // --- Category tabs (Mihon library tab row) ---
    m_categoryTabs = new QTabBar(this);
    m_categoryTabs->setExpanding(false);
    m_categoryTabs->setDrawBase(false);
    m_categoryTabs->setUsesScrollButtons(true);
    mainLayout->addWidget(m_categoryTabs);

    // --- Filter menu ---
    QMenu *filterMenu = new QMenu(this);
    QAction *dlAct = filterMenu->addAction("Downloaded only");
    dlAct->setCheckable(true);
    dlAct->setChecked(PreferenceManager::instance().value("downloadedOnly", false).toBool());
    QAction *unreadAct = filterMenu->addAction("Unread");
    unreadAct->setCheckable(true);
    connect(filterMenu, &QMenu::aboutToShow, this, [this, filterMenu]() {
        filterMenu->actions().first()->setChecked(
            PreferenceManager::instance().value("downloadedOnly", false).toBool());
        filterMenu->actions().at(1)->setChecked(m_unreadOnly);
    });
    connect(dlAct, &QAction::toggled, this, [this](bool on) {
        PreferenceManager::instance().setValue("downloadedOnly", on);
        applyFilters();
    });
    connect(unreadAct, &QAction::toggled, this, [this](bool on) {
        m_unreadOnly = on;
        applyFilters();
    });
    connect(m_filterBtn, &QPushButton::clicked, this, [this, filterMenu]() {
        filterMenu->popup(m_filterBtn->mapToGlobal(QPoint(0, m_filterBtn->height())));
    });

    // --- Wire-up ---
    connect(m_searchBtn, &QPushButton::clicked, this, [this]() {
        m_searchEdit->setVisible(m_searchEdit->isHidden());
        if (m_searchEdit->isVisible()) m_searchEdit->setFocus();
        else { m_searchEdit->clear(); m_searchQuery.clear(); applyFilters(); }
    });
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_searchQuery = text;
        applyFilters();
    });

    connect(m_categoryTabs, &QTabBar::currentChanged, this, [this](int index) {
        m_activeCategory = (index <= 0) ? -1
            : m_categoryTabs->tabData(index).toLongLong();
        applyFilters();
    });

    // --- Grid ---
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("background-color: transparent;");

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background-color: transparent;");

    m_gridLayout = new QGridLayout(m_contentWidget);
    m_gridLayout->setSpacing(12);
    m_gridLayout->setContentsMargins(16, 16, 16, 16);
    m_gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);

    m_emptyLabel = new QLabel("Your library is empty", m_contentWidget);
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    restyle();
}

void LibraryView::restyle()
{
    const M3Colors& c = ThemeManager::instance().colors();
    QLabel *title = this->findChild<QLabel*>("libraryTitle");
    if (title)
        title->setStyleSheet(
            QStringLiteral("font-size: 20px; font-weight: 600; color: %1; background: transparent;")
                .arg(c.onBackground.name()));
    m_emptyLabel->setStyleSheet(
        QStringLiteral("color: %1; font-size: 14px; padding: 60px; background: transparent;")
            .arg(c.onSurfaceVariant.name()));
}

void LibraryView::refreshLibrary()
{
    m_mangaList = DatabaseManager::instance().mangaRepository().getFavorites();

    // Rebuild category tabs: "All" + persisted categories
    QString currentTab;
    int cur = m_categoryTabs->currentIndex();
    if (cur >= 0) currentTab = m_categoryTabs->tabText(cur);

    while (m_categoryTabs->count() > 0)
        m_categoryTabs->removeTab(0);

    m_categoryTabs->addTab("All");
    m_categoryTabs->setTabData(0, QVariant());

    CategoryRepository& catRepo = DatabaseManager::instance().categoryRepository();
    const QList<Category> cats = catRepo.getAllCategories();
    for (int i = 0; i < cats.size(); ++i) {
        m_categoryTabs->addTab(cats[i].name());
        m_categoryTabs->setTabData(i + 1, QVariant::fromValue<qlonglong>(cats[i].id()));
    }
    // Restore selection
    for (int i = 0; i < m_categoryTabs->count(); ++i) {
        if (m_categoryTabs->tabText(i) == currentTab) { m_categoryTabs->setCurrentIndex(i); break; }
    }

    applyFilters();
}

void LibraryView::applyFilters()
{
    // Clear existing cards
    qDeleteAll(m_cards);
    m_cards.clear();

    ChapterRepository& chapterRepo = DatabaseManager::instance().chapterRepository();
    CategoryRepository& catRepo = DatabaseManager::instance().categoryRepository();

    for (const Manga& manga : m_mangaList) {
        // Category filter (-1 = All)
        if (m_activeCategory > 0) {
            if (!catRepo.getCategoriesForManga(manga.id()).contains(m_activeCategory))
                continue;
        }

        // Unread filter
        int unread = chapterRepo.getUnreadCountByMangaId(manga.id());
        if (m_unreadOnly && unread == 0)
            continue;

        // Downloaded-only filter (chapter URL not starting with "http" = downloaded)
        const QList<Chapter> chaps = chapterRepo.getChaptersByMangaId(manga.id());
        int downloadCount = 0;
        for (const auto& ch : chaps) {
            if (!ch.url().startsWith("http"))
                ++downloadCount;
        }
        if (m_downloadedOnly && downloadCount == 0)
            continue;

        // Search filter
        if (!m_searchQuery.isEmpty() &&
            !manga.title().contains(m_searchQuery, Qt::CaseInsensitive))
            continue;

        QString langBadge = (manga.source() == 0) ? QStringLiteral("Local") : QString();
        MangaCoverCard *card = new MangaCoverCard(manga, unread, downloadCount, langBadge, m_contentWidget);
        connect(card, &MangaCoverCard::clicked, this, &LibraryView::mangaSelected);
        m_cards.append(card);
    }

    m_gridLayout->addWidget(m_emptyLabel, 0, 0);
    m_emptyLabel->setVisible(m_cards.isEmpty());
    updateGridLayout();
}

void LibraryView::updateGridLayout()
{
    // Remove all from layout without deleting
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        m_gridLayout->takeAt(i);
    }

    if (m_cards.isEmpty()) {
        m_gridLayout->addWidget(m_emptyLabel, 0, 0);
        return;
    }

    int cardWidth = 128 + 12; // Width + spacing
    int availableWidth = m_scrollArea->viewport()->width();
    int columns = qMax(1, availableWidth / cardWidth);

    for (int i = 0; i < m_cards.size(); ++i) {
        m_gridLayout->addWidget(m_cards[i], i / columns, i % columns);
    }
}

void LibraryView::keyPressEvent(QKeyEvent *event)
{
    // Arrow-key grid navigation across manga cards; Enter opens (handled by card)
    if (m_cards.isEmpty()) {
        QWidget::keyPressEvent(event);
        return;
    }

    // Compute current focus index
    QWidget *focus = focusWidget();
    int current = -1;
    for (int i = 0; i < m_cards.size(); ++i)
        if (m_cards[i] == focus) { current = i; break; }
    if (current < 0) {
        current = 0;
        m_cards[current]->setFocus();
        event->accept();
        return;
    }

    int columns = qMax(1, m_scrollArea->viewport()->width() / (128 + 12));
    int target = current;
    switch (event->key()) {
    case Qt::Key_Left:  target = qMax(0, current - 1); break;
    case Qt::Key_Right: target = qMin(m_cards.size() - 1, current + 1); break;
    case Qt::Key_Up:    target = qMax(0, current - columns); break;
    case Qt::Key_Down:  target = qMin(m_cards.size() - 1, current + columns); break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    if (target != current) {
        m_cards[target]->setFocus();
        m_scrollArea->ensureWidgetVisible(m_cards[target]);
    }
    event->accept();
}

void LibraryView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGridLayout();
}

void LibraryView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}
