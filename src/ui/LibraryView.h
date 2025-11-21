#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QWidget>
#include <QListWidget>
#include "database/MangaRepository.h"

class LibraryView : public QWidget
{
    Q_OBJECT
public:
    explicit LibraryView(QWidget *parent = nullptr);
    void refreshLibrary();

signals:
    void mangaSelected(const Manga& manga);

private:
    void setupUi();
    QListWidget *m_libraryListWidget;
    QList<Manga> m_libraryManga;
};

#endif // LIBRARYVIEW_H
