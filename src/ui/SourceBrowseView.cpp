#include "SourceBrowseView.h"
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
}

void SourceBrowseView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header Toolbar
    QWidget *header = new QWidget(this);
    header->setStyleSheet("background-color: #3B4252; padding: 10px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);

    QPushButton *backBtn = new QPushButton("Back", this);
    connect(backBtn, &QPushButton::clicked, this, &SourceBrowseView::backRequested);
    headerLayout->addWidget(backBtn);

    m_sourceNameLabel = new QLabel("Source", this);
    m_sourceNameLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #ECEFF4;");
    headerLayout->addWidget(m_sourceNameLabel);
    headerLayout->addStretch();

    mainLayout->addWidget(header);

    // Search and Filter Bar
    QWidget *searchBar = new QWidget(this);
    searchBar->setStyleSheet("background-color: #2E3440; padding: 10px; border-bottom: 1px solid #3B4252;");
    QVBoxLayout *searchLayout = new QVBoxLayout(searchBar);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search...");
    m_searchEdit->setStyleSheet("padding: 8px; background-color: #3B4252; color: white; border: none; border-radius: 4px;");
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SourceBrowseView::onSearchTriggered);
    searchLayout->addWidget(m_searchEdit);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    m_popularBtn = new QPushButton("Popular", this);
    m_latestBtn = new QPushButton("Latest", this);

    m_popularBtn->setStyleSheet("padding: 5px 15px; background-color: #4C566A; border-radius: 4px;");
    m_latestBtn->setStyleSheet("padding: 5px 15px; background-color: #4C566A; border-radius: 4px;");

    connect(m_popularBtn, &QPushButton::clicked, this, &SourceBrowseView::onPopularClicked);
    connect(m_latestBtn, &QPushButton::clicked, this, &SourceBrowseView::onLatestClicked);

    filterLayout->addWidget(m_popularBtn);
    filterLayout->addWidget(m_latestBtn);
    filterLayout->addStretch();
    searchLayout->addLayout(filterLayout);

    mainLayout->addWidget(searchBar);

    // Progress Bar and Status Label
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // indeterminate
    m_progressBar->setFixedHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet("QProgressBar { border: none; background: #2E3440; } QProgressBar::chunk { background: #88C0D0; }");
    m_progressBar->hide();
    mainLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #D8DEE9; padding: 10px; font-size: 14px;");
    m_statusLabel->hide();
    mainLayout->addWidget(m_statusLabel);

    // Content: Manga List
    m_mangaListView = new MangaListView(this);
    connect(m_mangaListView, &MangaListView::mangaSelected, this, &SourceBrowseView::mangaSelected);
    connect(m_mangaListView, &MangaListView::backRequested, this, &SourceBrowseView::backRequested);
    mainLayout->addWidget(m_mangaListView);

    connect(&m_watcher, &QFutureWatcher<QList<Manga>>::finished, this, &SourceBrowseView::onMangaFetchFinished);
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

    QString query = m_searchEdit->text();
    m_progressBar->show();
    m_statusLabel->setText(QString("Searching \"%1\"...").arg(query));
    m_statusLabel->show();
    m_mangaListView->hide();
    m_popularBtn->setEnabled(false);
    m_latestBtn->setEnabled(false);

    SourceBase *src = m_source;
    QFuture<QList<Manga>> future = QtConcurrent::run([src, query]() {
        return src->getSearchManga(query);
    });
    m_watcher.setFuture(future);
}

void SourceBrowseView::onPopularClicked()
{
    if (!m_source || m_watcher.isRunning()) return;

    m_progressBar->show();
    m_statusLabel->setText("Loading popular manga...");
    m_statusLabel->show();
    m_mangaListView->hide();
    m_popularBtn->setEnabled(false);
    m_latestBtn->setEnabled(false);

    SourceBase *src = m_source;
    QFuture<QList<Manga>> future = QtConcurrent::run([src]() {
        return src->getPopularManga();
    });
    m_watcher.setFuture(future);
}

void SourceBrowseView::onLatestClicked()
{
    if (!m_source || m_watcher.isRunning()) return;

    m_progressBar->show();
    m_statusLabel->setText("Loading latest updates...");
    m_statusLabel->show();
    m_mangaListView->hide();
    m_popularBtn->setEnabled(false);
    m_latestBtn->setEnabled(false);

    SourceBase *src = m_source;
    QFuture<QList<Manga>> future = QtConcurrent::run([src]() {
        return src->getLatestUpdates();
    });
    m_watcher.setFuture(future);
}

void SourceBrowseView::onMangaFetchFinished()
{
    m_progressBar->hide();
    m_statusLabel->hide();
    m_mangaListView->show();
    m_popularBtn->setEnabled(true);
    m_latestBtn->setEnabled(m_source ? m_source->supportsLatest() : true);
    QList<Manga> list = m_watcher.result();
    qDebug() << "SourceBrowseView: Successfully loaded and populated" << list.size() << "manga items into grid view.";
    m_mangaListView->populateManga(list);
}
