#ifndef EXTENSIONMANAGERVIEW_H
#define EXTENSIONMANAGERVIEW_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>

class ExtensionManagerView : public QWidget
{
    Q_OBJECT
public:
    explicit ExtensionManagerView(QWidget *parent = nullptr);

signals:
    void backRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onAddRepoClicked();
    void refreshAvailableExtensions();
    void onCustomContextMenuRequested(const QPoint& pos);

private:
    void setupUi();
    
    QListWidget *m_availableList;
    QLineEdit *m_repoUrlEdit;
};

#endif // EXTENSIONMANAGERVIEW_H
