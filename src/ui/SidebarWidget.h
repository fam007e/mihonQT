#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);

signals:
    void navigationRequested(int index); // 0: Library, 1: Updates, 2: History, 3: Browse, 4: More

private:
    QVBoxLayout *m_layout;
    QToolButton *m_libraryBtn;
    QToolButton *m_updatesBtn;
    QToolButton *m_historyBtn;
    QToolButton *m_browseBtn;
    QToolButton *m_moreBtn;

    void setupUi();
    QToolButton* createNavButton(const QString &text, const QString &iconName, int index);
};

#endif // SIDEBARWIDGET_H
