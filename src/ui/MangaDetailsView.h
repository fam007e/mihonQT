#ifndef MANGADETAILSVIEW_H
#define MANGADETAILSVIEW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include "model/Manga.h"
#include "model/Chapter.h"

class MangaDetailsView : public QWidget
{
    Q_OBJECT
public:
    explicit MangaDetailsView(QWidget *parent = nullptr);
    void setManga(const Manga& manga);

signals:
    void chapterSelected(const Manga& manga, const Chapter& chapter);
    void backRequested();
    void libraryStatusChanged(); // Signal when added/removed from library

private:
    void setupUi();
    void refreshChapters();
    void updateLibraryButton();

    Manga m_manga;
    QList<Chapter> m_chapters;

    QLabel *m_titleLabel;
    QLabel *m_authorLabel;
    QLabel *m_descriptionLabel;
    QPushButton *m_libraryButton;
    QListWidget *m_chapterListWidget;
};

#endif // MANGADETAILSVIEW_H
