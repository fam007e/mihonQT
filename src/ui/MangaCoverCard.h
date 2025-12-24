#ifndef MANGACOVERCARD_H
#define MANGACOVERCARD_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include "../model/Manga.h"

class MangaCoverCard : public QFrame
{
    Q_OBJECT

public:
    explicit MangaCoverCard(const Manga& manga, int unreadCount = 0, QWidget *parent = nullptr);

    void setUnreadCount(int count);
    void setCoverImage(const QPixmap& pixmap);
    const Manga& manga() const { return m_manga; }

signals:
    void clicked(const Manga& manga);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUi();
    void updateBadge();

    Manga m_manga;
    int m_unreadCount;

    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_badgeLabel;
};

#endif // MANGACOVERCARD_H
