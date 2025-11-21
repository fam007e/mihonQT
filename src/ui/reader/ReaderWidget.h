#ifndef READERWIDGET_H
#define READERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QVector>
#include <QPainter>
#include "../../model/Manga.h" // Include Manga.h directly
#include "../../database/MangaRepository.h" // Include MangaRepository.h
#include "../../database/ChapterRepository.h" // Include ChapterRepository.h
#include <quazip/quazip.h> // For QuaZip
#include <quazip/quazipfile.h> // For QuaZipFile

// Forward declarations
class Chapter;
class LocalSource;

class ReaderWidget : public QWidget
{
    Q_OBJECT

public:
    enum ReadingMode {
        Webtoon,
        LeftToRight,
        RightToLeft,
        DoublePageSpread // Right-to-Left Double Page
    };
    Q_ENUM(ReadingMode)

    explicit ReaderWidget(QWidget *parent = nullptr);
    ~ReaderWidget();

    void loadChapter(long mangaId, long chapterId);
    void loadManga(const Manga& manga);
    void setReadingMode(ReadingMode mode);

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void navigateBack();
    void pageLoaded(const QImage& image, int pageIndex); // Updated signal

public slots:
    void displayPage(const QImage& image, int pageIndex);

private:
    void clearChapterContent();
    void loadAndDisplayPages(const Chapter& chapter);
    void setupWebtoonView();
    void setupPagedView();
    void updateView();
    void nextPage();
    void previousPage();

    QVBoxLayout *m_mainLayout;
    
    // Webtoon View
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;

    // Paged View
    QLabel *m_pagedLabel;
    int m_currentPageIndex;
    QVector<QImage> m_loadedPages; // Store loaded pages for paged view

    ReadingMode m_readingMode;
    Manga m_currentManga;
    QVector<Chapter> m_chapters;

    LocalSource *m_localSource;
    ChapterRepository *m_chapterRepo;
    MangaRepository *m_mangaRepo;

};

#endif // READERWIDGET_H
