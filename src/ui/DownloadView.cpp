#include "DownloadView.h"
#include <QMessageBox>

DownloadView::DownloadView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    // Connect to DownloadManager signals
    connect(&DownloadManager::instance(), &DownloadManager::downloadQueued,
            this, &DownloadView::onDownloadQueued);
    connect(&DownloadManager::instance(), &DownloadManager::downloadProgress,
            this, &DownloadView::onDownloadProgress);
    connect(&DownloadManager::instance(), &DownloadManager::downloadComplete,
            this, &DownloadView::onDownloadComplete);
    connect(&DownloadManager::instance(), &DownloadManager::queueChanged,
            this, &DownloadView::refreshQueue);

    refreshQueue();
}

void DownloadView::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Downloads", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("color: #88C0D0;");

    m_cancelAllBtn = new QPushButton("Cancel All", this);
    m_cancelAllBtn->setFixedWidth(100);
    connect(m_cancelAllBtn, &QPushButton::clicked, this, &DownloadView::onCancelAllClicked);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(m_statusLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_cancelAllBtn);
    layout->addLayout(headerLayout);

    // List
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setStyleSheet(
        "QListWidget { background-color: #2E3440; border: none; }"
        "QListWidget::item { padding: 12px; border-bottom: 1px solid #3B4252; }"
        "QListWidget::item:alternate { background-color: #3B4252; }"
    );
    layout->addWidget(m_listWidget);
}

void DownloadView::refreshQueue()
{
    m_listWidget->clear();
    m_itemMap.clear();

    QList<DownloadItem> queue = DownloadManager::instance().getQueue();
    DownloadItem current = DownloadManager::instance().getCurrentDownload();

    // Add current download first
    if (current.chapterId > 0) {
        QString text = QString("⬇️ %1 - %2\n   Downloading... %3/%4 pages")
            .arg(current.mangaTitle)
            .arg(current.chapterName)
            .arg(current.downloadedPages)
            .arg(current.totalPages);

        QListWidgetItem *item = new QListWidgetItem(text, m_listWidget);
        item->setForeground(QColor("#A3BE8C")); // Green for active
        m_itemMap[current.chapterId] = item;
    }

    // Add queued items
    for (const DownloadItem& dl : queue) {
        QString text = QString("⏳ %1 - %2\n   Queued")
            .arg(dl.mangaTitle)
            .arg(dl.chapterName);

        QListWidgetItem *item = new QListWidgetItem(text, m_listWidget);
        m_itemMap[dl.chapterId] = item;
    }

    if (m_listWidget->count() == 0) {
        QListWidgetItem *emptyItem = new QListWidgetItem("No downloads in queue", m_listWidget);
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
    }

    // Update status
    int total = queue.size() + (current.chapterId > 0 ? 1 : 0);
    if (total > 0) {
        m_statusLabel->setText(QString("%1 in queue").arg(total));
    } else {
        m_statusLabel->setText("");
    }
}

void DownloadView::onDownloadQueued(const DownloadItem& item)
{
    QString text = QString("⏳ %1 - %2\n   Queued")
        .arg(item.mangaTitle)
        .arg(item.chapterName);

    QListWidgetItem *listItem = new QListWidgetItem(text, m_listWidget);
    m_itemMap[item.chapterId] = listItem;
}

void DownloadView::onDownloadProgress(long chapterId, int current, int total)
{
    updateListItem(chapterId, current, total);
}

void DownloadView::onDownloadComplete(long chapterId)
{
    if (m_itemMap.contains(chapterId)) {
        QListWidgetItem *item = m_itemMap[chapterId];
        item->setText(item->text().split("\n").first() + "\n   ✅ Complete!");
        item->setForeground(QColor("#88C0D0"));

        // Remove after a short delay (would use QTimer in production)
        m_itemMap.remove(chapterId);
    }
}

void DownloadView::updateListItem(long chapterId, int current, int total)
{
    if (m_itemMap.contains(chapterId)) {
        QListWidgetItem *item = m_itemMap[chapterId];
        QString title = item->text().split("\n").first();
        title.replace("⏳", "⬇️");
        item->setText(QString("%1\n   Downloading... %2/%3 pages").arg(title).arg(current).arg(total));
        item->setForeground(QColor("#A3BE8C"));
    }
}

void DownloadView::onCancelAllClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Cancel Downloads",
        "Cancel all downloads in queue?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        DownloadManager::instance().cancelAllDownloads();
        refreshQueue();
    }
}
