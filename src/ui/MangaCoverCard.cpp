#include "MangaCoverCard.h"
#include "../util/CoverCache.h"
#include "ThemeManager.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QFontMetrics>
#include <QVBoxLayout>

// Static rounded-corner cover pixmap (Mihon clips covers to a small shape radius).
static QPixmap roundedCover(const QPixmap& src, int w, int h, int radius)
{
    QPixmap scaled = src.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap out(w, h);
    out.fill(Qt::transparent);

    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, w, h, radius, radius);
    p.setClipPath(path);
    p.drawPixmap((w - scaled.width()) / 2, (h - scaled.height()) / 2, scaled);
    p.end();
    return out;
}

MangaCoverCard::MangaCoverCard(const Manga& manga, int unreadCount, int downloadCount,
                               const QString& languageBadge, QWidget *parent)
    : QFrame(parent)
    , m_manga(manga)
    , m_unreadCount(unreadCount)
    , m_downloadCount(downloadCount)
    , m_languageBadge(languageBadge)
{
    setupUi();
    loadCover();
    connect(&CoverCache::instance(), &CoverCache::loaded, this, [this](const QString& url, const QPixmap& pm) {
        if (url == m_manga.thumbnailUrl())
            setCoverImage(pm);
    });
}

void MangaCoverCard::setupUi()
{
    setFrameShape(QFrame::NoFrame);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus); // keyboard navigation (arrows move, Enter opens)
    setFixedSize(CoverW + 8, CoverH + 8 + 34); // cover + margins + two-line title

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 0);
    layout->setSpacing(3);

    // --- Cover container ---
    QWidget *coverContainer = new QWidget(this);
    coverContainer->setFixedSize(CoverW + 4, CoverH + 4);

    m_coverLabel = new QLabel(coverContainer);
    m_coverLabel->setGeometry(2, 2, CoverW, CoverH);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    // Placeholder while loading: dim surface container
    QPixmap placeholder(CoverW, CoverH);
    placeholder.fill(ThemeManager::instance().colors().surfaceContainerHigh);
    m_coverLabel->setPixmap(roundedCover(placeholder, CoverW, CoverH, 6));

    // Badge chips (Mihon: badges sit on top of the cover)
    m_badgeStartLabel = new QLabel(coverContainer);
    m_badgeStartLabel->move(2, 2);
    m_badgeStartLabel->setAlignment(Qt::AlignCenter);

    m_badgeEndLabel = new QLabel(coverContainer);
    m_badgeEndLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(coverContainer);

    // --- Title below the cover (Mihon comfortable grid) ---
    m_titleLabel = new QLabel(this);
    m_titleLabel->setWordWrap(false); // we do manual 2-line eliding
    m_titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_titleLabel->setFixedHeight(30);
    layout->addWidget(m_titleLabel);

    updateTitle();
    updateBadges();
    restyle();
}

void MangaCoverCard::restyle()
{
    const M3Colors& c = ThemeManager::instance().colors();

    // Badges: unread = secondary, downloads = tertiary (Mihon LibraryBadges.kt)
    QString badgeQss = QStringLiteral(
        "QLabel { border-radius: 4px; font-size: 10px; font-weight: 600; padding: 2px 5px; }");
    if (m_downloadCount > 0 && m_unreadCount > 0) {
        // Two chips side by side: draw as colored text spans
        m_badgeStartLabel->setText(QStringLiteral(
            "<span style=\"background-color:%1; color:%2;\">&nbsp;%3&nbsp;</span>"
            "<span style=\"background-color:%4; color:%5;\">&nbsp;%6&nbsp;</span>")
            .arg(c.tertiary.name(), c.onTertiary.name(), QString::number(m_downloadCount),
                 c.secondary.name(), c.onSecondary.name(), QString::number(m_unreadCount)));
        m_badgeStartLabel->setStyleSheet(badgeQss + "QLabel { background: transparent; }");
    } else if (m_unreadCount > 0) {
        m_badgeStartLabel->setText(QString::number(m_unreadCount));
        m_badgeStartLabel->setStyleSheet(badgeQss +
            QStringLiteral("QLabel { background-color: %1; color: %2; }")
                .arg(c.secondary.name(), c.onSecondary.name()));
    } else if (m_downloadCount > 0) {
        m_badgeStartLabel->setText(QString::number(m_downloadCount));
        m_badgeStartLabel->setStyleSheet(badgeQss +
            QStringLiteral("QLabel { background-color: %1; color: %2; }")
                .arg(c.tertiary.name(), c.onTertiary.name()));
    }
    m_badgeStartLabel->adjustSize();

    // Language badge (top-end, tertiary like Mihon LanguageBadge)
    if (!m_languageBadge.isEmpty()) {
        m_badgeEndLabel->setText(m_languageBadge.toUpper());
        m_badgeEndLabel->setStyleSheet(badgeQss +
            QStringLiteral("QLabel { background-color: %1; color: %2; }")
                .arg(c.tertiary.name(), c.onTertiary.name()));
        m_badgeEndLabel->adjustSize();
        m_badgeEndLabel->move(width() - m_badgeEndLabel->width() - 4, 2);
        m_badgeEndLabel->show();
    } else {
        m_badgeEndLabel->hide();
    }

    m_titleLabel->setStyleSheet(
        QStringLiteral("color: %1; font-size: 12px;").arg(c.onSurface.name()));
}

void MangaCoverCard::updateTitle()
{
    const QFontMetrics fm(m_titleLabel->font());
    QString t = m_manga.title();
    // Manual 2-line elide
    if (fm.horizontalAdvance(t) <= CoverW) {
        m_titleLabel->setText(t);
        return;
    }
    // Split roughly in half for two lines, elide second
    int mid = qBound(1, t.length() / 2, t.length() - 1);
    int breakAt = t.lastIndexOf(' ', mid);
    if (breakAt <= 0) breakAt = mid;
    QString line1 = t.left(breakAt).trimmed();
    QString rest = t.mid(breakAt).trimmed();
    QString line2 = rest.length() > CoverW
        ? fm.elidedText(rest, Qt::ElideRight, CoverW)
        : rest;
    m_titleLabel->setText(line1 + "\n" + line2);
}

void MangaCoverCard::updateBadges()
{
    restyle();
}

void MangaCoverCard::loadCover()
{
    QString thumb = m_manga.thumbnailUrl();
    if (thumb.isEmpty() && m_manga.source() == 0) {
        // Local-source fallback: look for a cover file in the manga directory
        QDir dir(m_manga.url());
        QStringList filters = {"cover.*", "folder.*", "poster.*", "*.jpg", "*.png", "*.jpeg"};
        QFileInfoList list = dir.entryInfoList(filters, QDir::Files, QDir::Name);
        if (!list.isEmpty())
            thumb = list.first().absoluteFilePath();
    }
    if (thumb.isEmpty())
        return;

    if (thumb.startsWith("http://") || thumb.startsWith("https://")) {
        // Image CDNs with hotlink protection want a Referer from the site
        // the cover belongs to; the manga page URL's origin is the best hint.
        QString referer;
        const QUrl mangaUrl(m_manga.url());
        if (mangaUrl.isValid() && (mangaUrl.scheme() == "http" || mangaUrl.scheme() == "https")) {
            referer = mangaUrl.scheme() + "://" + mangaUrl.host() + "/";
        }
        QPixmap cached = CoverCache::instance().load(thumb, referer);
        if (!cached.isNull())
            setCoverImage(cached);
    } else {
        QPixmap pm(thumb);
        if (!pm.isNull())
            setCoverImage(pm);
    }
}

void MangaCoverCard::setUnreadCount(int count)
{
    m_unreadCount = count;
    updateBadges();
}

void MangaCoverCard::setDownloadCount(int count)
{
    m_downloadCount = count;
    updateBadges();
}

void MangaCoverCard::setCoverImage(const QPixmap& pixmap)
{
    if (!pixmap.isNull())
        m_coverLabel->setPixmap(roundedCover(pixmap, CoverW, CoverH, 6));
}

void MangaCoverCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(m_manga);
    QFrame::mousePressEvent(event);
}

void MangaCoverCard::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
        emit clicked(m_manga);
        event->accept();
        return;
    default:
        QFrame::keyPressEvent(event);
    }
}

void MangaCoverCard::enterEvent(QEnterEvent *event)
{
    // Subtle lift like Mihon's cover press/hover feedback
    const M3Colors& c = ThemeManager::instance().colors();
    setStyleSheet(QStringLiteral("MangaCoverCard { background-color: %1; border-radius: 10px; }")
                      .arg(ThemeManager::rgba(c.onSurface, 0.08f)));
    QFrame::enterEvent(event);
}

void MangaCoverCard::leaveEvent(QEvent *event)
{
    setStyleSheet("MangaCoverCard { background: transparent; }");
    QFrame::leaveEvent(event);
}
