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
#include <QDateTime>
#include <QSqlQuery>

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
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // cards wrap instead
    scroll->setStyleSheet("background-color: transparent;");

    QWidget *content = new QWidget();
    m_statsLayout = new QVBoxLayout(content);
    m_statsLayout->setAlignment(Qt::AlignTop);
    m_statsLayout->setSpacing(15);

    // Library overview: what the user has and how much of it is read.
    QGridLayout *libraryGrid = addSection("Library", 4);
    libraryGrid->addWidget(createStatCard("manga", "Manga", ":/icons/library.svg"), 0, 0);
    libraryGrid->addWidget(createStatCard("chapters", "Chapters", ":/icons/browse.svg"), 0, 1);
    libraryGrid->addWidget(createStatCard("read", "Read", ":/icons/history.svg"), 0, 2);
    libraryGrid->addWidget(createStatCard("unread", "Unread", ":/icons/browse.svg"), 0, 3);

    // Reading state: how far along the library is, manga by manga.
    QGridLayout *stateGrid = addSection("Reading state", 3);
    stateGrid->addWidget(createStatCard("started", "Started", ":/icons/library.svg"), 0, 0);
    stateGrid->addWidget(createStatCard("completed", "Completed", ":/icons/library.svg"), 0, 1);
    stateGrid->addWidget(createStatCard("categories", "Categories", ":/icons/categories.svg"), 0, 2);

    // Activity: recent behaviour, from the reading history.
    QGridLayout *activityGrid = addSection("Activity", 3);
    activityGrid->addWidget(createStatCard("last7", "Last 7 days", ":/icons/history.svg"), 0, 0);
    activityGrid->addWidget(createStatCard("last30", "Last 30 days", ":/icons/history.svg"), 0, 1);
    activityGrid->addWidget(createStatCard("lastRead", "Last read", ":/icons/history.svg"), 0, 2);

    // Outside library: content synced from browsing that was never added.
    QGridLayout *browsedGrid = addSection("Outside library", 3);
    browsedGrid->addWidget(createStatCard("browsedManga", "Mangas", ":/icons/browse.svg"), 0, 0);
    browsedGrid->addWidget(createStatCard("browsedChapters", "Chapters", ":/icons/browse.svg"), 0, 1);
    browsedGrid->addWidget(createStatCard("browsedRead", "Read", ":/icons/history.svg"), 0, 2);

    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

void StatisticsView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshStats(); // keep the dashboard current every time it is opened
}

QGridLayout* StatisticsView::addSection(const QString& title, int columns)
{
    QLabel *sectionLabel = new QLabel(title);
    sectionLabel->setObjectName("sectionTitle");
    m_statsLayout->addWidget(sectionLabel);

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(12);
    for (int c = 0; c < columns; ++c)
        grid->setColumnStretch(c, 1);
    m_statsLayout->addLayout(grid);
    return grid;
}

QWidget* StatisticsView::createStatCard(const QString& key, const QString& title, const QString& icon)
{
    QFrame *card = new QFrame();
    card->setObjectName("statCard");

    QHBoxLayout *layout = new QHBoxLayout(card);

    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(icon).pixmap(32, 32));
    iconLabel->setFixedSize(32, 32);

    QVBoxLayout *textLayout = new QVBoxLayout();
    QLabel *valueLabel = new QLabel("0");
    valueLabel->setObjectName("valueLabel");
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("statTitle");
    titleLabel->setWordWrap(true); // keeps 4-column rows usable at narrow widths
    textLayout->addWidget(valueLabel);
    textLayout->addWidget(titleLabel);

    layout->addWidget(iconLabel);
    layout->addLayout(textLayout);
    layout->addStretch();

    m_valueLabels.insert(key, valueLabel);
    return card;
}

void StatisticsView::setValue(const QString& key, int value)
{
    setValue(key, QString::number(value));
}

void StatisticsView::setValue(const QString& key, const QString& value)
{
    if (m_valueLabels.contains(key))
        m_valueLabels[key]->setText(value);
}

int StatisticsView::countQuery(const QString& sql)
{
    QSqlQuery query(sql);
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}

QString StatisticsView::relativeDate(long long epochSecs)
{
    if (epochSecs <= 0) return QStringLiteral("—");
    qint64 secs = QDateTime::currentSecsSinceEpoch() - epochSecs;
    if (secs < 0) secs = 0;
    if (secs < 60) return QStringLiteral("just now");
    if (secs < 3600) return QStringLiteral("%1h ago").arg(secs / 3600);
    if (secs < 86400) return QStringLiteral("%1d ago").arg(secs / 86400);
    if (secs < 86400LL * 30) return QStringLiteral("%1w ago").arg(secs / (86400LL * 7));
    return QDateTime::fromSecsSinceEpoch(epochSecs).date().toString("MMM d, yyyy");
}

void StatisticsView::refreshStats()
{
    auto& db = DatabaseManager::instance();
    const qint64 now = QDateTime::currentSecsSinceEpoch();

    // The database also holds mangas/chapters synced from browsing that were
    // never added to the library — the Library/Reading state/Activity sections
    // count the library only; "Outside library" reports the rest.
    const int totalChapters = db.chapterRepository().getTotalChapterCount();
    const int readChapters = db.chapterRepository().getReadChapterCount();

    // Library overview
    setValue("manga", db.mangaRepository().getFavorites().size());
    setValue("chapters", totalChapters);
    setValue("read", readChapters);
    setValue("unread", qMax(0, totalChapters - readChapters));

    // Reading state: started = at least one chapter read; completed = chapters
    // exist and none are unread.
    setValue("started", countQuery(QStringLiteral(
        "SELECT COUNT(DISTINCT c.manga_id) FROM chapters c "
        "JOIN mangas m ON c.manga_id = m._id WHERE m.favorite = 1 AND c.read = 1")));
    setValue("completed", countQuery(QStringLiteral(
        "SELECT COUNT(*) FROM mangas m WHERE m.favorite = 1 "
        "AND EXISTS (SELECT 1 FROM chapters c WHERE c.manga_id = m._id) "
        "AND NOT EXISTS (SELECT 1 FROM chapters c WHERE c.manga_id = m._id AND c.read = 0)")));
    setValue("categories", db.categoryRepository().getAllCategories().size());

    // Activity: chapters whose history timestamp falls in the window.
    setValue("last7", countQuery(QStringLiteral(
        "SELECT COUNT(*) FROM history h "
        "JOIN chapters c ON h.chapter_id = c._id "
        "JOIN mangas m ON c.manga_id = m._id "
        "WHERE m.favorite = 1 AND h.last_read >= %1").arg(now - 7 * 86400)));
    setValue("last30", countQuery(QStringLiteral(
        "SELECT COUNT(*) FROM history h "
        "JOIN chapters c ON h.chapter_id = c._id "
        "JOIN mangas m ON c.manga_id = m._id "
        "WHERE m.favorite = 1 AND h.last_read >= %1").arg(now - 30 * 86400)));
    const long long lastRead = static_cast<long long>(countQuery(QStringLiteral(
        "SELECT COALESCE(MAX(h.last_read), 0) FROM history h "
        "JOIN chapters c ON h.chapter_id = c._id "
        "JOIN mangas m ON c.manga_id = m._id WHERE m.favorite = 1")));
    setValue("lastRead", relativeDate(lastRead));

    // Outside library: what browsing synced that was never added.
    setValue("browsedManga", countQuery(
        "SELECT COUNT(*) FROM mangas WHERE favorite = 0"));
    setValue("browsedChapters", countQuery(QStringLiteral(
        "SELECT COUNT(*) FROM chapters c "
        "JOIN mangas m ON c.manga_id = m._id WHERE m.favorite = 0")));
    setValue("browsedRead", countQuery(QStringLiteral(
        "SELECT COUNT(*) FROM chapters c "
        "JOIN mangas m ON c.manga_id = m._id WHERE m.favorite = 0 AND c.read = 1")));
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

    QList<QLabel*> sectionTitles = findChildren<QLabel*>("sectionTitle");
    for (QLabel* section : sectionTitles) {
        section->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 600;").arg(statTitleColor));
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
