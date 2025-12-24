#include "MangaListView.h"
#include "MangaCoverCard.h"
#include <QVBoxLayout>
#include <QResizeEvent>

MangaListView::MangaListView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void MangaListView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("background-color: transparent;");

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background-color: transparent;");

    m_gridLayout = new QGridLayout(m_contentWidget);
    m_gridLayout->setSpacing(15);
    m_gridLayout->setContentsMargins(15, 15, 15, 15);
    m_gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
}

void MangaListView::populateManga(const QList<Manga>& mangaList)
{
    // Clear existing cards
    qDeleteAll(m_cards);
    m_cards.clear();

    m_mangaList = mangaList;

    for (const Manga& manga : m_mangaList) {
        MangaCoverCard *card = new MangaCoverCard(manga, 0, m_contentWidget);
        connect(card, &MangaCoverCard::clicked, this, &MangaListView::mangaSelected);
        m_cards.append(card);
    }

    updateGridLayout();
}

void MangaListView::updateGridLayout()
{
    // Remove all from layout without deleting
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        m_gridLayout->takeAt(i);
    }

    int cardWidth = 130 + 15;
    int availableWidth = m_scrollArea->viewport()->width();
    int columns = qMax(1, availableWidth / cardWidth);

    for (int i = 0; i < m_cards.size(); ++i) {
        int row = i / columns;
        int col = i % columns;
        m_gridLayout->addWidget(m_cards[i], row, col);
    }
}

void MangaListView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGridLayout();
}

void MangaListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}
