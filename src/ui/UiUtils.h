#ifndef UIUTILS_H
#define UIUTILS_H

#include <QListWidget>

class UiUtils
{
public:
    static void applyListWidgetStyle(QListWidget *listWidget)
    {
        listWidget->setAlternatingRowColors(true);
        listWidget->setStyleSheet(
            "QListWidget { background-color: #2E3440; border: none; outline: none; }"
            "QListWidget::item { padding: 12px; border-bottom: 1px solid #3B4252; color: #ECEFF4; }"
            "QListWidget::item:alternate { background-color: #3B4252; }"
            "QListWidget::item:selected { background-color: #4C566A; color: #88C0D0; }"
            "QListWidget::item:hover { background-color: #434C5E; }"
        );
    }

    static void setPlaceholderText(QListWidget *listWidget, const QString &text)
    {
        listWidget->clear();
        QListWidgetItem *item = new QListWidgetItem(text, listWidget);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setForeground(QColor("#6B7280"));
    }
};

#endif // UIUTILS_H
