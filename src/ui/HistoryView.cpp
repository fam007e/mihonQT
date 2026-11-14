#include "HistoryView.h"
#include "UiUtils.h"
#include "../database/DatabaseManager.h"
#include <QDateTime>
#include <QMessageBox>

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    refreshHistory();
}

void HistoryView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("History", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    m_clearAllBtn = new QPushButton("Clear All", this);
    // size to hint: a fixed width truncated the label with padding
    m_clearAllBtn->setMinimumWidth(m_clearAllBtn->sizeHint().width() + 12);
    connect(m_clearAllBtn, &QPushButton::clicked, this, &HistoryView::onClearAllClicked);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_clearAllBtn);
    layout->addLayout(headerLayout);

    // List
    m_listWidget = new QListWidget(this);
    UiUtils::applyListWidgetStyle(m_listWidget);
    connect(m_listWidget, &QListWidget::itemClicked, this, &HistoryView::onItemClicked);
    connect(m_listWidget, &QListWidget::itemActivated, this, &HistoryView::onItemClicked); // Enter key too
    layout->addWidget(m_listWidget);
}

void HistoryView::refreshHistory()
{
    m_listWidget->clear();
    m_entries = DatabaseManager::instance().historyRepository().getRecentHistory(100);

    if (m_entries.isEmpty()) {
        UiUtils::setPlaceholderText(m_listWidget, "No reading history yet");
        return;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        const HistoryEntry& entry = m_entries[i];

        QDateTime dt = QDateTime::fromSecsSinceEpoch(entry.lastRead);
        QString timeStr = dt.toString("MMM d, hh:mm AP");

        QString text = QString("%1\n%2 • %3")
            .arg(entry.mangaTitle)
            .arg(entry.chapterName)
            .arg(timeStr);

        QListWidgetItem *item = new QListWidgetItem(text, m_listWidget);
        item->setData(Qt::UserRole, i); // Store index
    }
}

void HistoryView::onItemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_entries.size()) {
        const HistoryEntry& entry = m_entries[index];
        emit resumeReading(entry.mangaId, entry.chapterId);
    }
}

void HistoryView::onClearAllClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Clear History",
        "Are you sure you want to clear all reading history?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        DatabaseManager::instance().historyRepository().deleteAllHistory();
        refreshHistory();
    }
}
