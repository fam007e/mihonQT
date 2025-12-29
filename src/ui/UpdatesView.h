#ifndef UPDATESVIEW_H
#define UPDATESVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "../database/ChapterRepository.h"
#include "../database/MangaRepository.h"

struct UpdateEntry {
    long mangaId;
    long chapterId;
    QString mangaTitle;
    QString chapterName;
    QString thumbnailUrl;
    long dateFetch;
    bool read; // flawfinder: ignore
};

class UpdatesView : public QWidget
{
    Q_OBJECT

public:
    explicit UpdatesView(QWidget *parent = nullptr);
    void refreshUpdates();

signals:
    void chapterSelected(long mangaId, long chapterId);
    void mangaSelected(long mangaId);

private slots:
    void onItemClicked(QListWidgetItem *item);

private:
    void setupUi();

    QListWidget *m_listWidget;

    QList<UpdateEntry> m_entries;
};

#endif // UPDATESVIEW_H
