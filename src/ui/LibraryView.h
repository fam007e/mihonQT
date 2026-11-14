#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QTabBar>
#include <QMenu>
#include <QLabel>
#include <QKeyEvent>
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
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUi();
    void updateGridLayout();
    void applyFilters();
    void restyle();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QGridLayout *m_gridLayout;
    QList<MangaCoverCard*> m_cards;
    QList<Manga> m_mangaList;

    QPushButton *m_searchBtn;
    QPushButton *m_filterBtn;
    QPushButton *m_moreBtn;
    QLineEdit *m_searchEdit;
    QTabBar *m_categoryTabs;
    QLabel *m_emptyLabel;

    // Filters
    QString m_searchQuery;
    bool m_downloadedOnly = false;
    bool m_unreadOnly = false;
    long m_activeCategory = -1; // -1 = All
};

#endif // LIBRARYVIEW_H
