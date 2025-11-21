#include "CategoryEditDialog.h"
#include "../config/PreferenceManager.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>

CategoryEditDialog::CategoryEditDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Edit Categories");
    resize(400, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_categoryList = new QListWidget(this);
    mainLayout->addWidget(m_categoryList);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QVBoxLayout *actionLayout = new QVBoxLayout();
    m_addButton = new QPushButton("Add", this);
    m_renameButton = new QPushButton("Rename", this);
    m_deleteButton = new QPushButton("Delete", this);
    
    actionLayout->addWidget(m_addButton);
    actionLayout->addWidget(m_renameButton);
    actionLayout->addWidget(m_deleteButton);
    actionLayout->addStretch();

    QVBoxLayout *moveLayout = new QVBoxLayout();
    m_moveUpButton = new QPushButton("Move Up", this);
    m_moveDownButton = new QPushButton("Move Down", this);
    
    moveLayout->addWidget(m_moveUpButton);
    moveLayout->addWidget(m_moveDownButton);
    moveLayout->addStretch();

    buttonLayout->addLayout(actionLayout);
    buttonLayout->addLayout(moveLayout);
    mainLayout->addLayout(buttonLayout);

    QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("Save", this);
    m_cancelButton = new QPushButton("Cancel", this);
    
    dialogButtonLayout->addStretch();
    dialogButtonLayout->addWidget(m_saveButton);
    dialogButtonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(dialogButtonLayout);

    connect(m_addButton, &QPushButton::clicked, this, &CategoryEditDialog::onAddClicked);
    connect(m_renameButton, &QPushButton::clicked, this, &CategoryEditDialog::onRenameClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &CategoryEditDialog::onDeleteClicked);
    connect(m_moveUpButton, &QPushButton::clicked, this, &CategoryEditDialog::onMoveUpClicked);
    connect(m_moveDownButton, &QPushButton::clicked, this, &CategoryEditDialog::onMoveDownClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &CategoryEditDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_categoryList, &QListWidget::currentRowChanged, this, &CategoryEditDialog::updateButtons);

    loadCategories();
    updateButtons();
}

void CategoryEditDialog::loadCategories()
{
    QStringList categories = PreferenceManager::instance().value("library/categories", QStringList() << "Default").toStringList();
    m_categoryList->addItems(categories);
}

void CategoryEditDialog::onAddClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add Category",
                                         "Category Name:", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        // Check for duplicates
        if (m_categoryList->findItems(text, Qt::MatchExactly).isEmpty()) {
            m_categoryList->addItem(text);
            updateButtons();
        } else {
            QMessageBox::warning(this, "Error", "Category already exists.");
        }
    }
}

void CategoryEditDialog::onRenameClicked()
{
    QListWidgetItem *item = m_categoryList->currentItem();
    if (!item) return;

    bool ok;
    QString text = QInputDialog::getText(this, "Rename Category",
                                         "Category Name:", QLineEdit::Normal,
                                         item->text(), &ok);
    if (ok && !text.isEmpty()) {
         if (m_categoryList->findItems(text, Qt::MatchExactly).isEmpty() || text == item->text()) {
            item->setText(text);
        } else {
            QMessageBox::warning(this, "Error", "Category already exists.");
        }
    }
}

void CategoryEditDialog::onDeleteClicked()
{
    QListWidgetItem *item = m_categoryList->currentItem();
    if (!item) return;

    if (m_categoryList->count() <= 1) {
        QMessageBox::warning(this, "Error", "You must have at least one category.");
        return;
    }

    delete m_categoryList->takeItem(m_categoryList->row(item));
    updateButtons();
}

void CategoryEditDialog::onMoveUpClicked()
{
    int row = m_categoryList->currentRow();
    if (row > 0) {
        QListWidgetItem *item = m_categoryList->takeItem(row);
        m_categoryList->insertItem(row - 1, item);
        m_categoryList->setCurrentRow(row - 1);
    }
}

void CategoryEditDialog::onMoveDownClicked()
{
    int row = m_categoryList->currentRow();
    if (row < m_categoryList->count() - 1) {
        QListWidgetItem *item = m_categoryList->takeItem(row);
        m_categoryList->insertItem(row + 1, item);
        m_categoryList->setCurrentRow(row + 1);
    }
}

void CategoryEditDialog::onSaveClicked()
{
    QStringList categories;
    for (int i = 0; i < m_categoryList->count(); ++i) {
        categories << m_categoryList->item(i)->text();
    }
    PreferenceManager::instance().setValue("library/categories", categories);
    accept();
}

void CategoryEditDialog::updateButtons()
{
    bool hasSelection = m_categoryList->currentItem() != nullptr;
    m_renameButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection && m_categoryList->count() > 1);
    
    int row = m_categoryList->currentRow();
    m_moveUpButton->setEnabled(hasSelection && row > 0);
    m_moveDownButton->setEnabled(hasSelection && row < m_categoryList->count() - 1);
}
