#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);

signals:
    void navigationRequested(int index); // 0: Library, 1: Updates, 2: History, 3: Browse, 4: More

private:
    QVBoxLayout *m_layout;
    QPushButton *m_libraryBtn;
    QPushButton *m_updatesBtn;
    QPushButton *m_historyBtn;
    QPushButton *m_browseBtn;
    QPushButton *m_settingsBtn;

    void setupUi();
    QPushButton* createNavButton(const QString &text, const QString &iconName, int index);
};

#endif // SIDEBARWIDGET_H
