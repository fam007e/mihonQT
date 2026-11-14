#ifndef MANGADETAILSVIEW_H
#define MANGADETAILSVIEW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPixmap>
#include <QKeyEvent>
#include "model/Manga.h"
#include "model/Chapter.h"

class ChapterRow; // defined in MangaDetailsView.cpp

// Mihon-style manga details screen:
//  - cover + title/author/status header
//  - centered action chips (library toggle, web view, categories)
//  - chapter list with Mihon chapter rows (unread dot, date/scanlator
//    subtitle, download indicator) inside one scrollable column
class MangaDetailsView : public QWidget
{
    Q_OBJECT
public:
    explicit MangaDetailsView(QWidget *parent = nullptr);
    void setManga(const Manga& manga);

signals:
    void chapterSelected(const Manga& manga, const Chapter& chapter);
    void libraryStatusChanged(); // Signal when added/removed from library
    void backRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupUi();
    QList<ChapterRow*> chapterRows() const;
    void refreshChapters();
    void updateLibraryButton();
    void updateSortButton();
    void updateHeader();
    void restyle();

    Manga m_manga;
    QList<Chapter> m_chapters;
    enum ChapterFilter { All, UnreadOnly, DownloadedOnly, BookmarkedOnly };
    ChapterFilter m_chapterFilter = All;

    // Header widgets
    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_authorLabel;
    QLabel *m_statusLabel;
    QLabel *m_chapterCountLabel;
    QLabel *m_descriptionLabel;
    QPushButton *m_libraryButton;
    QPushButton *m_categoriesButton;
    QPushButton *m_openWebViewButton;

    // Chapter section
    QPushButton *m_sortButton;
    QPushButton *m_filterButton;
    QWidget *m_chapterListContainer;
    QVBoxLayout *m_chapterListLayout;
    QLabel *m_chaptersHeaderLabel;

    QScrollArea *m_scrollArea;
};

#endif // MANGADETAILSVIEW_H
