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
class ReaderBarsOverlay;
class SourceManager; // Forward decl

class ReaderWidget : public QWidget
{
    Q_OBJECT

public:
    enum ReadingMode {
        Webtoon,
        LeftToRight,
        RightToLeft,
        DoublePageSpread, // Double page, manga-ordered: current page on the
                          // right, next on the left — always, in any reading
                          // mode (manga artwork reads right-to-left)
        DoublePageSpreadLeftToRight // Legacy persisted value only; normalized
                                    // to DoublePageSpread on apply
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
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

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
    void toggleDoublePageSpread();
    // Applies a mode selection from UI paths whose "Double Page" carries no
    // direction (settings default, overlay combo); resolves it against the
    // current reading context. Does not write the manga's saved mode.
    void applyReadingMode(int mode);

private:
    void clearChapterContent();
    void loadAndDisplayPages(const Chapter& chapter, bool startAtEnd = false);
    void loadOnlineChapter(const Chapter& chapter, bool startAtEnd = false); // New method
    void setupWebtoonView();
    void setupPagedView();
    void updateView();
    void updateWebtoonView();
    void nextPage();
    void previousPage();
    void saveReadingProgress();
    void onReadingModeChanged(int mode);
    ReadingMode defaultSinglePageMode() const;
    bool forwardIsLeft() const;
    void cycleReadingMode();
    // The single-page mode the reader was in before entering double-page
    // mode, so Shift+D is a true round trip (and heals a manga saved with
    // the opposite double-page orientation from older builds).
    ReadingMode m_modeBeforeDouble = LeftToRight;
    bool m_hasModeBeforeDouble = false;
    void onScaleTypeChanged(int type);
    void onFirstPageCoverChanged(bool enabled);
    void onAutoWideDetectChanged(bool enabled);
    void onBrightnessChanged(int value);
    void applyBrightness();
    void updateBarsOverlay();

    struct SpreadInfo {
        int firstIndex = -1;
        int secondIndex = -1;
        int stepForward = 1;
        bool isSingle = true;
    };

    SpreadInfo getSpreadInfo(int index) const;
    int getPreviousSpreadIndex(int currentIndex) const;

    QVBoxLayout *m_mainLayout;
    QStackedWidget *m_viewStack;

    // Webtoon View
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QVector<QLabel*> m_webtoonLabels;

    // Paged View
    QScrollArea *m_pagedScrollArea;
    QLabel *m_pagedLabel;
    int m_currentPageIndex;
    QVector<QImage> m_loadedPages;

    ReadingMode m_readingMode;
    ScaleType m_scaleType;
    int m_webtoonPadding;
    bool m_keepScreenOn;
    bool m_fullscreen;
    bool m_firstPageCover;
    bool m_autoWideDetect;

    Manga m_currentManga;
    QVector<Chapter> m_chapters;

    LocalSource *m_localSource;
    SourceManager *m_sourceManager; // New member
    ChapterRepository *m_chapterRepo;
    MangaRepository *m_mangaRepo;
    HistoryRepository *m_historyRepo;

    ReaderSettingsOverlay *m_settingsOverlay;
    ReaderBarsOverlay *m_barsOverlay = nullptr;
    QWidget *m_brightnessOverlay = nullptr; // dim layer (custom brightness)
    int m_brightness = 100;
    long m_currentChapterId = -1;
    bool m_showingTransition = false;

    bool hasNextChapter() const;
    bool hasPreviousChapter() const;
};

#endif // READERWIDGET_H
