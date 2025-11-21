#ifndef MANGALISTVIEW_H
#define MANGALISTVIEW_H

#include <QWidget>
#include <QListWidget>
#include <QList>
#include "model/Manga.h" // Include Manga header

class SourceBase; // Forward declaration

class MangaListView : public QWidget
{
    Q_OBJECT
public:
    explicit MangaListView(QWidget *parent = nullptr);

    void populateManga(const QList<Manga>& mangaList);

signals:
    void mangaSelected(const Manga& manga); // Changed signal

private:
    void setupUi();
    QListWidget *m_mangaListWidget;
    QList<Manga> m_mangaList; // Member to store the current manga list
};

#endif // MANGALISTVIEW_H