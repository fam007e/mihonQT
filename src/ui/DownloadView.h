#ifndef DOWNLOADVIEW_H
#define DOWNLOADVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "../download/DownloadManager.h"

class DownloadView : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadView(QWidget *parent = nullptr);
    void refreshQueue();

signals:
    void backRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onDownloadQueued(const DownloadItem& item);
    void onDownloadProgress(long chapterId, int current, int total);
    void onDownloadComplete(long chapterId);
    void onCancelAllClicked();

private:
    void setupUi();
    void updateListItem(long chapterId, int current, int total);

    QListWidget *m_listWidget;
    QPushButton *m_cancelAllBtn;
    QLabel *m_statusLabel;

    QMap<long, QListWidgetItem*> m_itemMap;
};

#endif // DOWNLOADVIEW_H
