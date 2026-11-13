#ifndef UIUTILS_H
#define UIUTILS_H

#include <QListWidget>
#include "ThemeManager.h"

class UiUtils
{
public:
    static void applyListWidgetStyle(QListWidget *listWidget)
    {
        listWidget->setAlternatingRowColors(false);
        listWidget->setStyleSheet(QStringLiteral(
            "QListWidget { background-color: transparent; border: none; outline: none; }"
            "QListWidget::item { padding: 12px; border-radius: 10px; color: %1; }"
            "QListWidget::item:hover { background-color: %2; }"
            "QListWidget::item:selected { background-color: %3; color: %4; }")
            .arg(ThemeManager::instance().colors().onSurface.name(),
                 ThemeManager::rgba(ThemeManager::instance().colors().onSurface, 0.10f),
                 ThemeManager::instance().colors().secondaryContainer.name(),
                 ThemeManager::instance().colors().onSecondaryContainer.name()));
    }

    static void setPlaceholderText(QListWidget *listWidget, const QString &text)
    {
        listWidget->clear();
        QListWidgetItem *item = new QListWidgetItem(text, listWidget);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setForeground(ThemeManager::instance().colors().onSurfaceVariant);
    }
};

#endif // UIUTILS_H
