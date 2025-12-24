#ifndef SOURCELISTVIEW_H
#define SOURCELISTVIEW_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>

#include "source/SourceManager.h"

class SourceListView : public QWidget
{
    Q_OBJECT
public:
    explicit SourceListView(SourceManager *sourceManager, QWidget *parent = nullptr);
    void refreshSources();

signals:
    void sourceSelected(long sourceId);

private slots:
    void onSourceClicked(QListWidgetItem *item);
    void onCustomContextMenuRequested(const QPoint &pos);

private:
    void setupUi();
    SourceManager *m_sourceManager;
    QListWidget *m_sourceListWidget;
};

#endif // SOURCELISTVIEW_H
