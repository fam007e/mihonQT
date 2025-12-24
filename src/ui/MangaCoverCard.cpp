#include "MangaCoverCard.h"
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <QFileInfo>

MangaCoverCard::MangaCoverCard(const Manga& manga, int unreadCount, QWidget *parent)
    : QFrame(parent)
    , m_manga(manga)
    , m_unreadCount(unreadCount)
{
    setupUi();

    QString thumb = m_manga.thumbnailUrl();
    if (thumb.isEmpty() && m_manga.source() == 0) {
        // Fallback for existing library items: search the manga directory
        QDir dir(m_manga.url());
        QStringList filters = {"cover.*", "folder.*", "poster.*", "*.jpg", "*.png", "*.jpeg"};
        QFileInfoList list = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        if (!list.isEmpty()) {
            thumb = list.first().absoluteFilePath();
        }
    }

    if (!thumb.isEmpty()) {
        QPixmap pixmap(thumb);
        if (!pixmap.isNull()) {
            setCoverImage(pixmap);
        }
    }
}

void MangaCoverCard::setupUi()
{
    setFixedSize(130, 190);
    setFrameShape(QFrame::NoFrame);
    setCursor(Qt::PointingHandCursor);

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(4);

    // Cover image container
    QWidget *coverContainer = new QWidget(this);
    coverContainer->setFixedSize(120, 160);
    coverContainer->setStyleSheet(
        "background-color: #3B4252;"
        "border-radius: 8px;"
    );

    // Cover label (inside container)
    m_coverLabel = new QLabel(coverContainer);
    m_coverLabel->setFixedSize(120, 160);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setScaledContents(true);
    m_coverLabel->setStyleSheet("border-radius: 8px;");

    // Title overlay at bottom of cover
    m_titleLabel = new QLabel(m_manga.title(), coverContainer);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_titleLabel->setFixedSize(120, 50);
    m_titleLabel->move(0, 110);
    m_titleLabel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "   stop:0 transparent, stop:0.3 rgba(0,0,0,0.7), stop:1 rgba(0,0,0,0.9));"
        "color: white;"
        "font-size: 11px;"
        "font-weight: bold;"
        "padding: 4px 6px;"
        "border-bottom-left-radius: 8px;"
        "border-bottom-right-radius: 8px;"
    );

    // Unread badge (top right corner)
    m_badgeLabel = new QLabel(coverContainer);
    m_badgeLabel->setAlignment(Qt::AlignCenter);
    m_badgeLabel->setFixedSize(28, 20);
    m_badgeLabel->move(88, 4);
    m_badgeLabel->setStyleSheet(
        "background-color: #88C0D0;"
        "color: #2E3440;"
        "font-size: 11px;"
        "font-weight: bold;"
        "border-radius: 4px;"
        "padding: 2px 4px;"
    );
    updateBadge();

    layout->addWidget(coverContainer);

    // Add shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(10);
    shadow->setOffset(2, 2);
    shadow->setColor(QColor(0, 0, 0, 80));
    coverContainer->setGraphicsEffect(shadow);

    // Hover effect via stylesheet
    setStyleSheet(
        "MangaCoverCard { background: transparent; }"
        "MangaCoverCard:hover { background-color: rgba(136, 192, 208, 0.1); border-radius: 8px; }"
    );
}

void MangaCoverCard::setUnreadCount(int count)
{
    m_unreadCount = count;
    updateBadge();
}

void MangaCoverCard::setCoverImage(const QPixmap& pixmap)
{
    if (!pixmap.isNull()) {
        m_coverLabel->setPixmap(pixmap.scaled(120, 160, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void MangaCoverCard::updateBadge()
{
    if (m_unreadCount > 0) {
        m_badgeLabel->setText(QString::number(m_unreadCount));
        m_badgeLabel->show();
    } else {
        m_badgeLabel->hide();
    }
}

void MangaCoverCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_manga);
    }
    QFrame::mousePressEvent(event);
}

void MangaCoverCard::enterEvent(QEnterEvent *event)
{
    // Optional: Add animation or visual feedback
    QFrame::enterEvent(event);
}

void MangaCoverCard::leaveEvent(QEvent *event)
{
    // Optional: Remove animation or visual feedback
    QFrame::leaveEvent(event);
}
