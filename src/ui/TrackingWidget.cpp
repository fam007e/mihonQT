#include "TrackingWidget.h"
#include "../tracking/AniListTracker.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>

TrackingWidget::TrackingWidget(long mangaId, QWidget *parent)
    : QWidget(parent)
    , m_mangaId(mangaId)
    , m_tracker(&AniListTracker::instance())
{
    setupUi();
    refreshState();

    connect(m_tracker, &TrackingService::loginSuccess, this, &TrackingWidget::refreshState);
    connect(m_tracker, &TrackingService::searchResults, this, &TrackingWidget::onSearchResults);
    connect(m_tracker, &TrackingService::bindSuccess, this, &TrackingWidget::refreshState);
    connect(m_tracker, &TrackingService::updateSuccess, this, [this](long) {
        QMessageBox::information(this, "Success", "Tracking updated!");
    });
    connect(m_tracker, &TrackingService::error, this, [this](const QString& msg) {
        QMessageBox::warning(this, "Error", msg);
    });
}

void TrackingWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // Header
    QLabel *titleLabel = new QLabel("Track with AniList", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout->addWidget(titleLabel);

    // Status
    m_statusLabel = new QLabel(this);
    layout->addWidget(m_statusLabel);

    // Login button
    m_loginBtn = new QPushButton("Login to AniList", this);
    connect(m_loginBtn, &QPushButton::clicked, this, &TrackingWidget::onLoginClicked);
    layout->addWidget(m_loginBtn);

    // Search section
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search manga on AniList...");
    m_searchBtn = new QPushButton("Search", this);
    connect(m_searchBtn, &QPushButton::clicked, this, &TrackingWidget::onSearchClicked);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);
    layout->addLayout(searchLayout);

    // Results list
    m_resultsList = new QListWidget(this);
    m_resultsList->setMaximumHeight(150);
    layout->addWidget(m_resultsList);

    // Bind button
    QPushButton *bindBtn = new QPushButton("Bind Selected", this);
    connect(bindBtn, &QPushButton::clicked, this, &TrackingWidget::onBindClicked);
    layout->addWidget(bindBtn);

    // Update section
    QHBoxLayout *updateLayout = new QHBoxLayout();

    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("Reading", 1);
    m_statusCombo->addItem("Completed", 2);
    m_statusCombo->addItem("On Hold", 3);
    m_statusCombo->addItem("Dropped", 4);
    m_statusCombo->addItem("Plan to Read", 5);

    m_chaptersSpin = new QSpinBox(this);
    m_chaptersSpin->setRange(0, 9999);
    m_chaptersSpin->setPrefix("Ch. ");

    m_updateBtn = new QPushButton("Update Progress", this);
    connect(m_updateBtn, &QPushButton::clicked, this, &TrackingWidget::onUpdateClicked);

    updateLayout->addWidget(new QLabel("Status:", this));
    updateLayout->addWidget(m_statusCombo);
    updateLayout->addWidget(m_chaptersSpin);
    updateLayout->addWidget(m_updateBtn);
    layout->addLayout(updateLayout);

    layout->addStretch();

    // Close button
    QPushButton *closeBtn = new QPushButton("Close", this);
    connect(closeBtn, &QPushButton::clicked, this, &TrackingWidget::closeRequested);
    layout->addWidget(closeBtn);
}

void TrackingWidget::setMangaId(long mangaId)
{
    m_mangaId = mangaId;
    refreshState();
}

void TrackingWidget::refreshState()
{
    bool loggedIn = m_tracker->isLoggedIn();

    m_loginBtn->setVisible(!loggedIn);
    m_searchEdit->setEnabled(loggedIn);
    m_searchBtn->setEnabled(loggedIn);
    m_statusCombo->setEnabled(loggedIn);
    m_chaptersSpin->setEnabled(loggedIn);
    m_updateBtn->setEnabled(loggedIn);

    if (loggedIn) {
        m_statusLabel->setText("✅ Logged in to AniList");
        m_statusLabel->setStyleSheet("color: #A3BE8C;");

        TrackingEntry entry = m_tracker->getEntry(m_mangaId);
        if (entry.id > 0) {
            m_chaptersSpin->setValue(entry.chaptersRead);
        }
    } else {
        m_statusLabel->setText("Not logged in");
        m_statusLabel->setStyleSheet("color: #BF616A;");
    }
}

void TrackingWidget::onLoginClicked()
{
    QDesktopServices::openUrl(m_tracker->authUrl());

    // Prompt for token (simplified - in production, use a proper callback handler)
    bool ok;
    QString token = QInputDialog::getText(this, "AniList Login",
        "After authorizing, paste the access_token from the URL:",
        QLineEdit::Normal, "", &ok);

    if (ok && !token.isEmpty()) {
        m_tracker->login(token);
    }
}

void TrackingWidget::onSearchClicked()
{
    QString query = m_searchEdit->text().trimmed();
    if (!query.isEmpty()) {
        m_tracker->search(query);
    }
}

void TrackingWidget::onSearchResults(const QList<TrackingSearchResult>& results)
{
    m_searchResults = results;
    m_resultsList->clear();

    for (const TrackingSearchResult& r : results) {
        QString text = QString("%1 (%2 chapters)").arg(r.title).arg(r.totalChapters);
        QListWidgetItem *item = new QListWidgetItem(text, m_resultsList);
        item->setData(Qt::UserRole, static_cast<qlonglong>(r.remoteId));
    }
}

void TrackingWidget::onBindClicked()
{
    QListWidgetItem *item = m_resultsList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Error", "Please select a manga to bind");
        return;
    }

    long remoteId = item->data(Qt::UserRole).toLongLong();
    m_tracker->bind(m_mangaId, remoteId);
}

void TrackingWidget::onUpdateClicked()
{
    int status = m_statusCombo->currentData().toInt();
    int chapters = m_chaptersSpin->value();

    m_tracker->update(m_mangaId, chapters, status, 0);
}
