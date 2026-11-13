#include "ReaderBarsOverlay.h"
#include "../ThemeManager.h"
#include <QResizeEvent>
#include <QFontMetrics>
#include <QPainter>

// Recolor a template SVG icon to the theme's foreground color
static QIcon tintedIcon(const QString& path, const QColor& color)
{
    QPixmap src(path);
    QPixmap tinted(src.size());
    tinted.fill(color);
    QPainter p(&tinted);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawPixmap(0, 0, src);
    p.end();
    return QIcon(tinted);
}

ReaderBarsOverlay::ReaderBarsOverlay(QWidget *parent)
    : QWidget(parent)
{
    // The overlay container is transparent to mouse events; only the
    // interactive children (buttons, slider) receive input.
    setAttribute(Qt::WA_TransparentForMouseEvents);

    const int barH = 52;

    // --- Top bar: back arrow | chapter title ---
    m_topBar = new QWidget(this);
    m_topBar->setFixedHeight(barH);

    QHBoxLayout *topLayout = new QHBoxLayout(m_topBar);
    topLayout->setContentsMargins(8, 0, 16, 0);
    topLayout->setSpacing(8);

    m_backBtn = new QPushButton("←", m_topBar);
    m_backBtn->setFixedSize(40, 40);
    m_backBtn->setFlat(true);
    m_backBtn->setProperty("flatIcon", true);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(m_backBtn);

    m_titleLabel = new QLabel(m_topBar);
    topLayout->addWidget(m_titleLabel, 1);

    // --- Bottom bar: prev chapter | page slider pill | next chapter ---
    // (Mihon ChapterNavigator horizontal layout)
    m_bottomBar = new QWidget(this);
    m_bottomBar->setFixedHeight(barH + 16);

    QHBoxLayout *bottomLayout = new QHBoxLayout(m_bottomBar);
    bottomLayout->setContentsMargins(8, 0, 8, 0);
    bottomLayout->setSpacing(8);

    m_prevChapterBtn = new QPushButton(m_bottomBar);
    m_prevChapterBtn->setIcon(QIcon(":/icons/skip_prev.svg"));
    m_prevChapterBtn->setFixedSize(44, 44);
    m_prevChapterBtn->setFlat(true);
    m_prevChapterBtn->setCursor(Qt::PointingHandCursor);
    m_prevChapterBtn->setToolTip("Previous chapter");
    bottomLayout->addWidget(m_prevChapterBtn);

    m_pageStartLabel = new QLabel("1", m_bottomBar);
    bottomLayout->addWidget(m_pageStartLabel);

    m_pageSlider = new QSlider(Qt::Horizontal, m_bottomBar);
    m_pageSlider->setRange(1, 1);
    bottomLayout->addWidget(m_pageSlider, 1);

    m_pageEndLabel = new QLabel("1", m_bottomBar);
    bottomLayout->addWidget(m_pageEndLabel);

    m_nextChapterBtn = new QPushButton(m_bottomBar);
    m_nextChapterBtn->setIcon(QIcon(":/icons/skip_next.svg"));
    m_nextChapterBtn->setFixedSize(44, 44);
    m_nextChapterBtn->setFlat(true);
    m_nextChapterBtn->setCursor(Qt::PointingHandCursor);
    m_nextChapterBtn->setToolTip("Next chapter");
    bottomLayout->addWidget(m_nextChapterBtn);

    connect(m_backBtn, &QPushButton::clicked, this, &ReaderBarsOverlay::backRequested);
    connect(m_prevChapterBtn, &QPushButton::clicked, this, &ReaderBarsOverlay::previousChapterRequested);
    connect(m_nextChapterBtn, &QPushButton::clicked, this, &ReaderBarsOverlay::nextChapterRequested);
    connect(m_pageSlider, &QSlider::valueChanged, this, [this](int value) {
        // Slider is 1-based; report 0-based page index
        int page = m_rtl ? (m_totalPages - value) : (value - 1);
        emit pageSelected(qBound(0, page, qMax(0, m_totalPages - 1)));
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
    restyle();
    hide();
}

void ReaderBarsOverlay::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_topBar->setGeometry(0, 0, width(), m_topBar->height());
    m_bottomBar->setGeometry(0, height() - m_bottomBar->height(), width(), m_bottomBar->height());
    // Re-elide title with the new width
    m_titleLabel->setText(QFontMetrics(m_titleLabel->font()).elidedText(
        m_titleLabel->toolTip().isEmpty() ? m_titleLabel->text() : m_titleLabel->toolTip(),
        Qt::ElideRight, qMax(100, width() - 160)));
}

void ReaderBarsOverlay::restyle()
{
    // Mihon: bars use surface color at 3dp elevation, alpha 0.9 (dark)
    const M3Colors& c = ThemeManager::instance().colors();
    const float alpha = 0.92f;
    QString barQss = QStringLiteral(
        "QWidget { background-color: %1; }")
        .arg(ThemeManager::rgba(c.surface, alpha));
    m_topBar->setStyleSheet(barQss);
    m_bottomBar->setStyleSheet(barQss);

    m_titleLabel->setStyleSheet(QStringLiteral(
        "background: transparent; color: %1; font-size: 14px; font-weight: 500;")
        .arg(c.onSurface.name()));
    m_pageStartLabel->setStyleSheet(QStringLiteral(
        "background: transparent; color: %1; font-size: 13px;")
        .arg(c.onSurface.name()));
    m_pageEndLabel->setStyleSheet(m_pageStartLabel->styleSheet());

    m_backBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; border: none; color: %1; font-size: 18px; }"
        "QPushButton:hover { background: rgba(127,127,127,0.2); border-radius: 20px; }")
        .arg(c.onSurface.name()));
    m_prevChapterBtn->setStyleSheet(m_backBtn->styleSheet());
    m_nextChapterBtn->setStyleSheet(m_backBtn->styleSheet());

    m_prevChapterBtn->setIcon(tintedIcon(":/icons/skip_prev.svg", c.onSurface));
    m_nextChapterBtn->setIcon(tintedIcon(":/icons/skip_next.svg", c.onSurface));
}

void ReaderBarsOverlay::setChapterTitle(const QString& title)
{
    QFontMetrics fm(m_titleLabel->font());
    m_titleLabel->setToolTip(title);
    m_titleLabel->setText(fm.elidedText(title, Qt::ElideRight, qMax(100, width() - 160)));
}

void ReaderBarsOverlay::setPageInfo(int current, int total)
{
    m_totalPages = qMax(1, total);
    m_pageEndLabel->setText(QString::number(m_totalPages));
    // Fixed-width start label so the slider doesn't shift as digits change
    m_pageStartLabel->setText(QString::number(m_rtl ? (m_totalPages - current) : (current + 1)));
    m_pageStartLabel->setMinimumWidth(m_pageEndLabel->width());

    m_pageSlider->blockSignals(true);
    m_pageSlider->setRange(1, m_totalPages);
    m_pageSlider->setValue(m_rtl ? (m_totalPages - current) : (current + 1));
    m_pageSlider->blockSignals(false);
}

void ReaderBarsOverlay::setRtl(bool rtl)
{
    m_rtl = rtl;
    m_pageSlider->setInvertedAppearance(rtl);
}

void ReaderBarsOverlay::setHasPreviousChapter(bool has)
{
    m_prevChapterBtn->setEnabled(has);
}

void ReaderBarsOverlay::setHasNextChapter(bool has)
{
    m_nextChapterBtn->setEnabled(has);
}
