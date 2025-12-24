#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include "model/Manga.h"

class MangaCoverCard;

class LibraryView : public QWidget
{
    Q_OBJECT
public:
    explicit LibraryView(QWidget *parent = nullptr);

    void refreshLibrary();

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

#endif // LIBRARYVIEW_H
