#ifndef MANGACOVERCARD_H
#define MANGACOVERCARD_H

#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QKeyEvent>
#include "../model/Manga.h"

// Mihon "comfortable grid" manga card:
//  - 2:3 rounded cover
//  - downloads + unread badges stacked at top-start, language badge at top-end
//  - title below the cover (2 lines, ellipsized), like Mihon's LibraryComfortableGrid
class MangaCoverCard : public QFrame
{
    Q_OBJECT

public:
    explicit MangaCoverCard(const Manga& manga, int unreadCount = 0,
                            int downloadCount = 0, const QString& languageBadge = QString(),
                            QWidget *parent = nullptr);

    void setUnreadCount(int count);
    void setDownloadCount(int count);
    void setCoverImage(const QPixmap& pixmap);
    const Manga& manga() const { return m_manga; }

signals:
    void clicked(const Manga& manga);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUi();
    void updateBadges();
    void updateTitle();
    void loadCover();
    void restyle();

    Manga m_manga;
    int m_unreadCount;
    int m_downloadCount;
    QString m_languageBadge;

    static constexpr int CoverW = 120;
    static constexpr int CoverH = 180;

    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_badgeStartLabel;   // "12"  or "12 3" (downloads unread combined chips)
    QLabel *m_badgeEndLabel;     // language / local badge
};

#endif // MANGACOVERCARD_H
