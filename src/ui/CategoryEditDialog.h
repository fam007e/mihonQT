#ifndef CATEGORYEDITDIALOG_H
#define CATEGORYEDITDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CategoryEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CategoryEditDialog(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onRenameClicked();
    void onDeleteClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onSaveClicked();

private:
    void loadCategories();
    void updateButtons();

    QListWidget *m_categoryList;
    QPushButton *m_addButton;
    QPushButton *m_renameButton;
    QPushButton *m_deleteButton;
    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif // CATEGORYEDITDIALOG_H
