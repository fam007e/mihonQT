#include "SourceBrowseView.h"
#include "ThemeManager.h"
#include "WebViewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QScrollArea>

SourceBrowseView::SourceBrowseView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
}

void SourceBrowseView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header Toolbar (Mihon source toolbar: back arrow | source name)
    QWidget *header = new QWidget(this);
    header->setFixedHeight(56);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 0, 16, 0);

    QPushButton *backBtn = new QPushButton("←", this);
    backBtn->setFlat(true);
    backBtn->setProperty("flatIcon", true);
    backBtn->setFixedSize(40, 40);
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked, this, &SourceBrowseView::backRequested);
    headerLayout->addWidget(backBtn);

    m_sourceNameLabel = new QLabel("Source", this);
    headerLayout->addWidget(m_sourceNameLabel, 1);

    mainLayout->addWidget(header);

    // Search and Filter Bar
    QWidget *searchBar = new QWidget(this);
    QVBoxLayout *searchLayout = new QVBoxLayout(searchBar);
    searchLayout->setContentsMargins(16, 4, 16, 10);
    searchLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search source...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(40);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SourceBrowseView::onSearchTriggered);
    searchLayout->addWidget(m_searchEdit);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    m_popularBtn = new QPushButton("Popular", this);
    m_latestBtn = new QPushButton("Latest", this);
    m_popularBtn->setCheckable(true);
    m_latestBtn->setCheckable(true);
    m_popularBtn->setCursor(Qt::PointingHandCursor);
    m_latestBtn->setCursor(Qt::PointingHandCursor);

    connect(m_popularBtn, &QPushButton::clicked, this, &SourceBrowseView::onPopularClicked);
    connect(m_latestBtn, &QPushButton::clicked, this, &SourceBrowseView::onLatestClicked);

    m_siteWebViewBtn = new QPushButton("Open in WebView", this);
    m_siteWebViewBtn->setCursor(Qt::PointingHandCursor);
    m_siteWebViewBtn->setToolTip("Open the source's site in a WebView — solve bot challenges manually; "
                                 "the cookies carry over to browsing");
    connect(m_siteWebViewBtn, &QPushButton::clicked, this, &SourceBrowseView::onOpenSiteWebView);

    filterLayout->addWidget(m_popularBtn);
    filterLayout->addWidget(m_latestBtn);
    filterLayout->addStretch();
    filterLayout->addWidget(m_siteWebViewBtn);
    searchLayout->addLayout(filterLayout);

    mainLayout->addWidget(searchBar);

    // Progress Bar and Status Label
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // indeterminate
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->hide();
    mainLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    mainLayout->addWidget(m_statusLabel);

    // Content: Manga List
    m_mangaListView = new MangaListView(this);
    connect(m_mangaListView, &MangaListView::mangaSelected, this, &SourceBrowseView::mangaSelected);
    connect(m_mangaListView, &MangaListView::backRequested, this, &SourceBrowseView::backRequested);
    mainLayout->addWidget(m_mangaListView);

    connect(&m_watcher, &QFutureWatcher<QList<Manga>>::finished, this, &SourceBrowseView::onMangaFetchFinished);
    connect(m_mangaListView, &MangaListView::bottomReached, this, &SourceBrowseView::requestNextPage);

    restyle();
}

void SourceBrowseView::restyle()
{
    const M3Colors& c = ThemeManager::instance().colors();
    m_sourceNameLabel->setStyleSheet(QStringLiteral(
        "background: transparent; font-weight: 600; font-size: 17px; color: %1;")
        .arg(c.onBackground.name()));
    m_statusLabel->setStyleSheet(QStringLiteral(
        "background: transparent; color: %1; padding: 10px; font-size: 14px;")
        .arg(c.onSurfaceVariant.name()));

    // Popular/Latest act as filter chips: checked = secondaryContainer pill
    QString chipQss = QStringLiteral(
        "QPushButton { background: transparent; border: 1px solid %1; border-radius: 20px;"
        "  color: %2; padding: 7px 18px; font-weight: 500; }"
        "QPushButton:hover { background: %3; }"
        "QPushButton:checked { background-color: %4; border-color: %4; color: %5; }")
        .arg(c.outlineVariant.name(), c.onSurface.name(),
             ThemeManager::rgba(c.onSurface, 0.08f),
             c.secondaryContainer.name(), c.onSecondaryContainer.name());
    m_popularBtn->setStyleSheet(chipQss);
    m_latestBtn->setStyleSheet(chipQss);
}

void SourceBrowseView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void SourceBrowseView::setSource(SourceBase *source)
{
    m_source = source;
    if (m_source) {
        m_sourceNameLabel->setText(m_source->name());
        m_latestBtn->setVisible(m_source->supportsLatest());
        onPopularClicked(); // Default to popular
    }
}

void SourceBrowseView::refresh()
{
    if (m_source) {
        onPopularClicked();
    }
}

void SourceBrowseView::onSearchTriggered()
{
    if (!m_source || m_watcher.isRunning()) return;

    m_popularBtn->setChecked(false);
    m_latestBtn->setChecked(false);

    m_currentQuery = m_searchEdit->text();
    startFetch(Mode::Search, 1);
}

void SourceBrowseView::onPopularClicked()
{
    if (!m_source || m_watcher.isRunning()) return;

    m_popularBtn->setChecked(true);
    m_latestBtn->setChecked(false);

    startFetch(Mode::Popular, 1);
}

void SourceBrowseView::onLatestClicked()
{
    if (!m_source || m_watcher.isRunning()) return;

    m_popularBtn->setChecked(false);
    m_latestBtn->setChecked(true);

    startFetch(Mode::Latest, 1);
}

// Fetch one page of the current listing. Page 1 shows the full-screen
// progress UI and replaces the grid; later pages append silently so the
// listing loads continuously like the Android app.
void SourceBrowseView::startFetch(Mode mode, int page)
{
    if (!m_source || m_watcher.isRunning()) return;

    m_mode = mode;
    m_currentPage = page;
    m_fullScreenLoad = (page == 1);
    if (page == 1) m_lastPageCount = -1; // fresh listing: clear the end marker

    SourceBase *src = m_source;
    QString query = m_currentQuery;
    QFuture<QList<Manga>> future;
    switch (mode) {
    case Mode::Popular:
        if (m_fullScreenLoad) m_statusLabel->setText("Loading popular manga...");
        future = QtConcurrent::run([src, page]() {
            return src->getPopularManga(page);
        });
        break;
    case Mode::Latest:
        if (m_fullScreenLoad) m_statusLabel->setText("Loading latest updates...");
        future = QtConcurrent::run([src, page]() {
            return src->getLatestUpdates(page);
        });
        break;
    case Mode::Search:
        if (m_fullScreenLoad) m_statusLabel->setText(QString("Searching \"%1\"...").arg(query));
        future = QtConcurrent::run([src, query, page]() {
            return src->getSearchManga(query, page);
        });
        break;
    }

    if (m_fullScreenLoad) {
        m_progressBar->show();
        m_statusLabel->show();
        m_mangaListView->hide();
        m_popularBtn->setEnabled(false);
        m_latestBtn->setEnabled(false);
    } else {
        m_statusLabel->setText("Loading more...");
        m_statusLabel->show();
    }

    m_watcher.setFuture(future);
}

// Android-style continuous loading: fetch the next page when the grid is
// scrolled near its bottom, until a page comes back empty.
void SourceBrowseView::requestNextPage()
{
    if (!m_source || m_watcher.isRunning()) return;
    if (m_lastPageCount == 0) return; // end of listing reached

    startFetch(m_mode, m_currentPage + 1);
}

// Manual WebView session for the source's site: the user can solve bot
// challenges at their own pace; harvested cookies seed the shared jar (see
// WebViewDialog::done) and the current listing reloads with them.
void SourceBrowseView::onOpenSiteWebView()
{
    if (!m_source) return;

    WebViewDialog dialog(m_source->baseUrl(), this);
    dialog.exec();

    if (!m_watcher.isRunning()) {
        startFetch(m_mode, 1); // refresh the listing with the new cookies
    }
}

void SourceBrowseView::onMangaFetchFinished()
{
    QList<Manga> list = m_watcher.result();
    m_lastPageCount = list.size();

    if (m_fullScreenLoad) {
        m_progressBar->hide();
        m_statusLabel->hide();
        m_mangaListView->show();
        m_popularBtn->setEnabled(true);
        m_latestBtn->setEnabled(m_source ? m_source->supportsLatest() : true);
        qDebug() << "SourceBrowseView: Successfully loaded and populated" << list.size() << "manga items into grid view.";
        m_mangaListView->populateManga(list);
    } else {
        m_statusLabel->hide();
        m_mangaListView->appendManga(list);
        qDebug() << "SourceBrowseView: Appended" << list.size() << "manga items (page" << m_currentPage << ").";
    }
}
