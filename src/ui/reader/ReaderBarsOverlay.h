#ifndef READERBARSOVERLAY_H
#define READERBARSOVERLAY_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

// Mihon reader chrome: translucent top bar (back + chapter title) and
// bottom bar (prev chapter | page slider "n / total" | next chapter).
// The container itself is mouse-transparent so page taps still work
// around the bars.
class ReaderBarsOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit ReaderBarsOverlay(QWidget *parent = nullptr);

    void setChapterTitle(const QString& title);
    void setPageInfo(int current, int total);
    void setRtl(bool rtl);              // invert slider direction for RTL manga
    void setHasPreviousChapter(bool has);
    void setHasNextChapter(bool has);

signals:
    void backRequested();
    void pageSelected(int pageIndex);   // 0-based
    void nextChapterRequested();
    void previousChapterRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void restyle();

    QWidget *m_topBar;
    QWidget *m_bottomBar;
    QPushButton *m_backBtn;
    QLabel *m_titleLabel;
    QLabel *m_pageStartLabel;
    QLabel *m_pageEndLabel;
    QSlider *m_pageSlider;
    QPushButton *m_prevChapterBtn;
    QPushButton *m_nextChapterBtn;
    int m_totalPages = 0;
    bool m_rtl = false;
};

#endif // READERBARSOVERLAY_H
