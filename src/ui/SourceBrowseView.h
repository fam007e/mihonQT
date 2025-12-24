#ifndef SOURCEBROWSEVIEW_H
#define SOURCEBROWSEVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
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

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUi();

    SourceBase *m_source = nullptr;

    QLabel *m_sourceNameLabel;
    QLineEdit *m_searchEdit;
    QPushButton *m_popularBtn;
    QPushButton *m_latestBtn;
    MangaListView *m_mangaListView;
};

#endif // SOURCEBROWSEVIEW_H
