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

    // Content: Manga List
    m_mangaListView = new MangaListView(this);
    connect(m_mangaListView, &MangaListView::mangaSelected, this, &SourceBrowseView::mangaSelected);
    connect(m_mangaListView, &MangaListView::backRequested, this, &SourceBrowseView::backRequested);
    mainLayout->addWidget(m_mangaListView);
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
    if (m_source) {
        QString query = m_searchEdit->text();
        m_mangaListView->populateManga(m_source->getSearchManga(query));
    }
}

void SourceBrowseView::onPopularClicked()
{
    if (m_source) {
        m_mangaListView->populateManga(m_source->getPopularManga());
    }
}

void SourceBrowseView::onLatestClicked()
{
    if (m_source) {
        m_mangaListView->populateManga(m_source->getLatestUpdates());
    }
}
