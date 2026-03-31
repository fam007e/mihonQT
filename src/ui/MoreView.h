#ifndef MOREVIEW_H
#define MOREVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QCheckBox>
#include <QMouseEvent>

class MoreView : public QWidget
{
    Q_OBJECT

public:
    explicit MoreView(QWidget *parent = nullptr);
signals:
    void settingsRequested();
    void extensionsRequested();
    void categoriesRequested();
    void statisticsRequested();
    void downloadsRequested();
    void dataStorageRequested();
    void aboutRequested();
    void helpRequested();
    void incognitoChanged(bool enabled);
    void downloadedOnlyChanged(bool enabled);
    void backRequested();

public slots:
    void updateThemeColors();
    void updatePreferences();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setupUi();
    void addActionItem(const QString& icon, const QString& title, const QString& subtitle, const std::function<void()>& callback);
    QCheckBox* addToggleItem(const QString& icon, const QString& title, const QString& subtitle, bool initialValue, const std::function<void(bool)>& callback);
    void addSeparator();

    QVBoxLayout *m_listLayout;
    QLabel *m_logoLabel;
    QLabel *m_qtBadge;
    QCheckBox *m_downloadedOnlyToggle;
    QCheckBox *m_incognitoToggle;
};

#endif // MOREVIEW_H
