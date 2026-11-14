#ifndef SOURCEBROWSEVIEW_H
#define SOURCEBROWSEVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QStackedWidget>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include "MangaListView.h"
#include "source/SourceBase.h"

class SourceBrowseView : public QWidget
{
    Q_OBJECT
public:
    explicit SourceBrowseView(QWidget *parent = nullptr);

    void setSource(SourceBase *source);
    void refresh();

signals:
    void backRequested();
    void mangaSelected(const Manga& manga);

private slots:
    void onSearchTriggered();
    void onPopularClicked();
    void onLatestClicked();
    void onOpenSiteWebView();
    void onMangaFetchFinished();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    // Browse mode for the current listing; pages are 1-based
    enum class Mode { Popular, Latest, Search };
    void startFetch(Mode mode, int page);
    void requestNextPage();

    void setupUi();
    void restyle();

    SourceBase *m_source = nullptr;

    QLabel *m_sourceNameLabel;
    QLineEdit *m_searchEdit;
    QPushButton *m_popularBtn;
    QPushButton *m_latestBtn;
    QPushButton *m_siteWebViewBtn;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    MangaListView *m_mangaListView;
    QFutureWatcher<QList<Manga>> m_watcher;

    Mode m_mode = Mode::Popular;
    QString m_currentQuery;
    int m_currentPage = 1;
    int m_lastPageCount = -1; // set to the fetched page's size; 0 stops paging
    bool m_fullScreenLoad = false; // page-1 load: progress bar + disabled chips
};

#endif // SOURCEBROWSEVIEW_H
