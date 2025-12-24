#include "UpdatesView.h"
#include "UiUtils.h"
#include "../database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

UpdatesView::UpdatesView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshUpdates();
}

void UpdatesView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // Header
    QLabel *titleLabel = new QLabel("Updates", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(titleLabel);

    // List
    m_listWidget = new QListWidget(this);
    UiUtils::applyListWidgetStyle(m_listWidget);
    connect(m_listWidget, &QListWidget::itemClicked, this, &UpdatesView::onItemClicked);
    layout->addWidget(m_listWidget);
}

void UpdatesView::refreshUpdates()
{
    m_listWidget->clear();
    m_entries.clear();

    // Query for recently fetched chapters from favorited manga
    QSqlQuery query;
    query.prepare(
        "SELECT c._id AS chapter_id, c.manga_id, c.name AS chapter_name, "
        "       c.date_fetch, c.read, "
        "       m.title AS manga_title, m.thumbnail_url "
        "FROM chapters c "
        "JOIN mangas m ON c.manga_id = m._id "
        "WHERE m.favorite = 1 "
        "ORDER BY c.date_fetch DESC "
        "LIMIT 100"
    );

    if (query.exec()) {
        while (query.next()) {
            UpdateEntry entry;
            entry.chapterId = query.value("chapter_id").toLongLong();
            entry.mangaId = query.value("manga_id").toLongLong();
            entry.mangaTitle = query.value("manga_title").toString();
            entry.chapterName = query.value("chapter_name").toString();
            entry.thumbnailUrl = query.value("thumbnail_url").toString();
            entry.dateFetch = query.value("date_fetch").toLongLong();
            entry.read = query.value("read").toBool();
            m_entries.append(entry);
        }
    } else {
        qDebug() << "Error querying updates:" << query.lastError().text();
    }

    if (m_entries.isEmpty()) {
        UiUtils::setPlaceholderText(m_listWidget, "No new updates");
        return;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        const UpdateEntry& entry = m_entries[i];

        QDateTime dt = QDateTime::fromSecsSinceEpoch(entry.dateFetch);
        QString timeStr = dt.toString("MMM d, hh:mm AP");

        QString readStatus = entry.read ? "✓" : "•";
        QString text = QString("%1 %2\n%3 • %4")
            .arg(readStatus)
            .arg(entry.mangaTitle)
            .arg(entry.chapterName)
            .arg(timeStr);

        QListWidgetItem *item = new QListWidgetItem(text, m_listWidget);
        item->setData(Qt::UserRole, i);

        if (entry.read) {
            item->setForeground(QColor("#6B7280")); // Dimmed for read
        }
    }
}

void UpdatesView::onItemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_entries.size()) {
        const UpdateEntry& entry = m_entries[index];
        emit chapterSelected(entry.mangaId, entry.chapterId);
    }
}
