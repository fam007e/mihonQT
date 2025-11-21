#ifndef SOURCELISTVIEW_H
#define SOURCELISTVIEW_H

#include <QWidget>
#include <QListWidget>

#include "source/SourceManager.h"

class SourceListView : public QWidget
{
    Q_OBJECT
public:
    explicit SourceListView(SourceManager *sourceManager, QWidget *parent = nullptr);

signals:
    void sourceSelected(long sourceId);

private:
    void setupUi();
    SourceManager *m_sourceManager;
    QListWidget *m_sourceListWidget;
};

#endif // SOURCELISTVIEW_H