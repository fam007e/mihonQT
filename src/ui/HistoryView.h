#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "../database/HistoryRepository.h"

class HistoryView : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryView(QWidget *parent = nullptr);
    void refreshHistory();

signals:
    void resumeReading(long mangaId, long chapterId);
    void mangaSelected(long mangaId);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onClearAllClicked();

private:
    void setupUi();

    QListWidget *m_listWidget;
    QPushButton *m_clearAllBtn;

    QList<HistoryEntry> m_entries;
};

#endif // HISTORYVIEW_H
