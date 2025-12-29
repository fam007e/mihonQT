#ifndef READERWIDGET_H
#define READERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QStackedWidget>
#include <QVector>
#include <QPainter>
#include "../../model/Manga.h"
#include "../../model/Chapter.h"
#include "../../database/MangaRepository.h"
#include "../../database/ChapterRepository.h"
#include "../../database/HistoryRepository.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

class LocalSource;
class ReaderSettingsOverlay;
class SourceManager; // Forward decl

class ReaderWidget : public QWidget
{
    Q_OBJECT

public:
    enum ReadingMode {
        Webtoon,
        LeftToRight,
        RightToLeft,
        DoublePageSpread, // Right-to-Left Double Page
        DoublePageSpreadLeftToRight // Left-to-Right Double Page
    };
    Q_ENUM(ReadingMode)

    enum ScaleType {
        FitScreen = 1,
        Stretch = 2,
        FitWidth = 3,
        FitHeight = 4,
        OriginalSize = 5,
        SmartFit = 6
    };
    Q_ENUM(ScaleType)

    explicit ReaderWidget(SourceManager* sourceManager, QWidget *parent = nullptr);
    ~ReaderWidget();

    void loadChapter(long mangaId, long chapterId, bool startAtEnd = false);
    void loadManga(const Manga& manga);
    void setReadingMode(ReadingMode mode);
    void reloadSettings();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void showSettingsOverlay();
    void hideSettingsOverlay();
    void toggleSettingsOverlay();

signals:
    void navigateBack();
    void pageLoaded(const QImage& image, int pageIndex);
    void chapterClosed(long chapterId, int lastPageRead);
    void requestNextChapter(long currentChapterId); // Signal to request loading the next chapter
    void requestPreviousChapter(long currentChapterId); // Signal to request loading the previous chapter

public slots:
    void displayPage(const QImage& image, int pageIndex);

private:
    void clearChapterContent();
    void loadAndDisplayPages(const Chapter& chapter, bool startAtEnd = false);
    void loadOnlineChapter(const Chapter& chapter, bool startAtEnd = false); // New method
    void setupWebtoonView();
    void setupPagedView();
    void updateView();
    void nextPage();
    void previousPage();
    void saveReadingProgress();
    void onReadingModeChanged(int mode);
    void onScaleTypeChanged(int type);

    QVBoxLayout *m_mainLayout;
    QStackedWidget *m_viewStack;

    // Webtoon View
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;

    // Paged View
    QScrollArea *m_pagedScrollArea;
    QLabel *m_pagedLabel;
    int m_currentPageIndex;
    QVector<QImage> m_loadedPages;

    ReadingMode m_readingMode;
    ReadingMode m_lastSinglePageMode; // To track preferred direction for Double Page switch
    ScaleType m_scaleType;
    int m_webtoonPadding;
    bool m_keepScreenOn;
    bool m_fullscreen;

    Manga m_currentManga;
    QVector<Chapter> m_chapters;

    LocalSource *m_localSource;
    SourceManager *m_sourceManager; // New member
    ChapterRepository *m_chapterRepo;
    MangaRepository *m_mangaRepo;
    HistoryRepository *m_historyRepo;

    ReaderSettingsOverlay *m_settingsOverlay;
    long m_currentChapterId = -1;
    bool m_showingTransition = false;

    bool hasNextChapter() const;
    bool hasPreviousChapter() const;
};

#endif // READERWIDGET_H
