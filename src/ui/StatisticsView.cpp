#include "StatisticsView.h"
#include "ThemeManager.h"
#include "../database/DatabaseManager.h"
#include "../database/MangaRepository.h"
#include "../database/ChapterRepository.h"
#include "../database/CategoryRepository.h"
#include "../model/Manga.h"
#include "../model/Category.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QIcon>
#include <QMouseEvent>

StatisticsView::StatisticsView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &StatisticsView::updateThemeColors);
    updateThemeColors(); // Initial apply
    refreshStats();
}

void StatisticsView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Statistics", this);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background-color: transparent;");

    QWidget *content = new QWidget();
    m_statsLayout = new QVBoxLayout(content);
    m_statsLayout->setAlignment(Qt::AlignTop);
    m_statsLayout->setSpacing(15);

    m_totalMangaLabel = new QLabel("0", this);
    m_statsLayout->addWidget(createStatCard("Total Manga", "0", ":/icons/library.svg"));
    
    m_totalChaptersLabel = new QLabel("0", this);
    m_statsLayout->addWidget(createStatCard("Total Chapters", "0", ":/icons/browse.svg"));

    m_readChaptersLabel = new QLabel("0", this);
    m_statsLayout->addWidget(createStatCard("Read Chapters", "0", ":/icons/history.svg"));

    m_totalCategoriesLabel = new QLabel("0", this);
    m_statsLayout->addWidget(createStatCard("Categories", "0", ":/icons/categories.svg"));

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

QWidget* StatisticsView::createStatCard(const QString& title, const QString& value, const QString& icon)
{
    QFrame *card = new QFrame();
    card->setObjectName("statCard");
    
    QHBoxLayout *layout = new QHBoxLayout(card);
    
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(icon).pixmap(32, 32));
    iconLabel->setFixedSize(32, 32);
    
    QVBoxLayout *textLayout = new QVBoxLayout();
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("statTitle");
    
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("valueLabel");
    
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(valueLabel);
    
    layout->addWidget(iconLabel);
    layout->addLayout(textLayout);
    layout->addStretch();
    
    return card;
}

void StatisticsView::refreshStats()
{
    auto& db = DatabaseManager::instance();
    
    int mangaCount = db.mangaRepository().getAllManga().size();
    int categoryCount = db.categoryRepository().getAllCategories().size();
    int totalChapters = db.chapterRepository().getTotalChapterCount();
    int readChapters = db.chapterRepository().getReadChapterCount();
    
    // Update labels inside cards
    QList<QLabel*> labels = findChildren<QLabel*>("valueLabel");
    if (labels.size() >= 4) {
        labels[0]->setText(QString::number(mangaCount));
        labels[1]->setText(QString::number(totalChapters));
        labels[2]->setText(QString::number(readChapters));
        labels[3]->setText(QString::number(categoryCount));
    }
}

void StatisticsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void StatisticsView::updateThemeColors()
{
    QPalette p = QApplication::palette();
    QString cardBg = p.color(QPalette::Button).name();
    QString titleColor = p.color(QPalette::WindowText).name();
    QString statTitleColor = p.color(QPalette::Text).name();
    QString valueColor = p.color(QPalette::Highlight).name();

    QLabel *titleLabel = findChild<QLabel*>("titleLabel");
    if (titleLabel) {
        titleLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(titleColor));
    }

    QList<QFrame*> cards = findChildren<QFrame*>("statCard");
    for (QFrame* card : cards) {
        card->setStyleSheet(QString("QFrame#statCard { background-color: %1; border-radius: 8px; padding: 15px; }").arg(cardBg));
        
        QLabel* sTitle = card->findChild<QLabel*>("statTitle");
        if (sTitle) sTitle->setStyleSheet(QString("color: %1; font-size: 14px;").arg(statTitleColor));
        
        QLabel* sValue = card->findChild<QLabel*>("valueLabel");
        if (sValue) sValue->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: bold;").arg(valueColor));
    }
}
