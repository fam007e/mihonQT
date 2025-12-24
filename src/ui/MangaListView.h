#ifndef MANGALISTVIEW_H
#define MANGALISTVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QList>
#include "model/Manga.h"

class MangaCoverCard;

class MangaListView : public QWidget
{
    Q_OBJECT
public:
    explicit MangaListView(QWidget *parent = nullptr);

    void populateManga(const QList<Manga>& mangaList);

signals:
    void mangaSelected(const Manga& manga);
    void backRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUi();
    void updateGridLayout();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QGridLayout *m_gridLayout;
    QList<MangaCoverCard*> m_cards;
    QList<Manga> m_mangaList;
};

#endif // MANGALISTVIEW_H
