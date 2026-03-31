#ifndef EXTENSIONSVIEW_H
#define EXTENSIONSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "ExtensionManager.h"

class ExtensionsView : public QWidget
{
    Q_OBJECT
public:
    explicit ExtensionsView(QWidget *parent = nullptr);

signals:
    void backRequested();
    void extensionsInstalled();

private slots:
    void refreshList();
    void addRepository();

private:
    void setupUi();
    
    ExtensionManager& m_extensionManager;
    QListWidget *m_installedList;
    QListWidget *m_availableList;
    QLineEdit *m_searchEdit;
};

#endif // EXTENSIONSVIEW_H
