#include "TrackingWidget.h"
#include "database/DatabaseManager.h"
#include "database/ChapterRepository.h"
#include "model/Chapter.h"
#include "../tracking/TrackerManager.h"
#include "../tracking/AniListTracker.h"
#include "../tracking/MyAnimeListTracker.h"
#include "../tracking/MangaUpdatesTracker.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>

TrackingWidget::TrackingWidget(long mangaId, const QString& mangaTitle, QWidget *parent)
    : QWidget(parent)
    , m_mangaId(mangaId)
    , m_mangaTitle(mangaTitle)
    , m_tracker(&AniListTracker::instance())
{
    setupUi();
    connectTrackerSignals();
    refreshState();

    if (!m_mangaTitle.isEmpty()) {
        m_searchEdit->setText(m_mangaTitle);
        if (m_tracker->isLoggedIn()) {
            m_tracker->search(m_mangaTitle);
        }
    }
}

void TrackingWidget::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // Header with tracker selection
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Track Progress", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    m_trackerCombo = new QComboBox(this);
    m_trackerCombo->setIconSize(QSize(20, 20));
    m_trackerCombo->addItem(QIcon(":/icons/anilist.svg"), "AniList", "anilist");
    m_trackerCombo->addItem(QIcon(":/icons/myanimelist.svg"), "MyAnimeList", "myanimelist");
    m_trackerCombo->addItem(QIcon(":/icons/mangaupdates.svg"), "MangaUpdates", "mangaupdates");
    connect(m_trackerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TrackingWidget::onTrackerChanged);
    headerLayout->addWidget(m_trackerCombo);
    layout->addLayout(headerLayout);

    // Status and Auth Buttons
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel(this);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();

    m_loginBtn = new QPushButton("Login", this);
    connect(m_loginBtn, &QPushButton::clicked, this, &TrackingWidget::onLoginClicked);
    statusLayout->addWidget(m_loginBtn);

    m_logoutBtn = new QPushButton("Logout", this);
    connect(m_logoutBtn, &QPushButton::clicked, this, &TrackingWidget::onLogoutClicked);
    statusLayout->addWidget(m_logoutBtn);
    layout->addLayout(statusLayout);

    // Search section
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search manga on tracker...");
    m_searchBtn = new QPushButton("Search", this);
    connect(m_searchBtn, &QPushButton::clicked, this, &TrackingWidget::onSearchClicked);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);
    layout->addLayout(searchLayout);

    // Results list
    m_resultsList = new QListWidget(this);
    m_resultsList->setMaximumHeight(160);
    connect(m_resultsList, &QListWidget::itemDoubleClicked, this, &TrackingWidget::onBindClicked);
    layout->addWidget(m_resultsList);

    // Bind / Unbind actions
    QHBoxLayout *bindLayout = new QHBoxLayout();
    m_bindBtn = new QPushButton("Bind Selected Manga", this);
    connect(m_bindBtn, &QPushButton::clicked, this, &TrackingWidget::onBindClicked);
    bindLayout->addWidget(m_bindBtn);

    m_unbindBtn = new QPushButton("Unbind", this);
    connect(m_unbindBtn, &QPushButton::clicked, this, &TrackingWidget::onUnbindClicked);
    bindLayout->addWidget(m_unbindBtn);
    layout->addLayout(bindLayout);

    // Progress update section
    QHBoxLayout *updateLayout = new QHBoxLayout();

    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItem("Reading", TrackingService::Reading);
    m_statusCombo->addItem("Completed", TrackingService::Completed);
    m_statusCombo->addItem("On Hold", TrackingService::OnHold);
    m_statusCombo->addItem("Dropped", TrackingService::Dropped);
    m_statusCombo->addItem("Plan to Read", TrackingService::PlanToRead);

    m_chaptersSpin = new QSpinBox(this);
    m_chaptersSpin->setRange(0, 9999);
    m_chaptersSpin->setPrefix("Ch. ");

    m_scoreSpin = new QDoubleSpinBox(this);
    m_scoreSpin->setRange(0.0, 10.0);
    m_scoreSpin->setSingleStep(1.0);
    m_scoreSpin->setDecimals(1);
    m_scoreSpin->setSpecialValueText("- (No rating)");
    m_scoreSpin->setPrefix("★ ");

    m_updateBtn = new QPushButton("Update Progress", this);
    connect(m_updateBtn, &QPushButton::clicked, this, &TrackingWidget::onUpdateClicked);

    updateLayout->addWidget(new QLabel("Status:", this));
    updateLayout->addWidget(m_statusCombo);
    updateLayout->addWidget(new QLabel("Chapters:", this));
    updateLayout->addWidget(m_chaptersSpin);
    updateLayout->addWidget(new QLabel("Score:", this));
    updateLayout->addWidget(m_scoreSpin);
    updateLayout->addWidget(m_updateBtn);
    layout->addLayout(updateLayout);

    layout->addStretch();

    // Close button
    QPushButton *closeBtn = new QPushButton("Close", this);
    connect(closeBtn, &QPushButton::clicked, this, &TrackingWidget::closeRequested);
    layout->addWidget(closeBtn);
}

void TrackingWidget::connectTrackerSignals()
{
    connect(m_tracker, &TrackingService::loginSuccess, this, [this]() {
        refreshState();
        if (!m_searchEdit->text().trimmed().isEmpty()) {
            m_tracker->search(m_searchEdit->text().trimmed());
        }
    });
    connect(m_tracker, &TrackingService::loginFailed, this, [this](const QString& err) {
        QMessageBox::warning(this, "Login Failed", err);
        refreshState();
    });
    connect(m_tracker, &TrackingService::searchResults, this, &TrackingWidget::onSearchResults);
    connect(m_tracker, &TrackingService::bindSuccess, this, &TrackingWidget::refreshState);
    connect(m_tracker, &TrackingService::updateSuccess, this, [this](long) {
        QMessageBox::information(this, "Success", "Tracking progress updated!");
        refreshState();
    });
    connect(m_tracker, &TrackingService::error, this, [this](const QString& msg) {
        QMessageBox::warning(this, "Tracker Error", msg);
    });
}

void TrackingWidget::onTrackerChanged(int index)
{
    if (m_tracker) {
        m_tracker->disconnect(this);
    }

    QString id = m_trackerCombo->itemData(index).toString();
    m_tracker = TrackerManager::instance().tracker(id);
    if (!m_tracker) {
        m_tracker = &AniListTracker::instance();
    }

    connectTrackerSignals();
    m_resultsList->clear();
    refreshState();

    if (!m_mangaTitle.isEmpty()) {
        m_searchEdit->setText(m_mangaTitle);
        if (m_tracker->isLoggedIn()) {
            m_tracker->search(m_mangaTitle);
        }
    }
}

void TrackingWidget::setManga(long mangaId, const QString& mangaTitle)
{
    m_mangaId = mangaId;
    m_mangaTitle = mangaTitle;
    if (!m_mangaTitle.isEmpty()) {
        m_searchEdit->setText(m_mangaTitle);
        if (m_tracker && m_tracker->isLoggedIn()) {
            m_tracker->search(m_mangaTitle);
        }
    }
    refreshState();
}

void TrackingWidget::setMangaId(long mangaId)
{
    setManga(mangaId, m_mangaTitle);
}

void TrackingWidget::refreshState()
{
    bool loggedIn = m_tracker->isLoggedIn();

    m_loginBtn->setVisible(!loggedIn);
    m_logoutBtn->setVisible(loggedIn);
    m_searchEdit->setEnabled(loggedIn);
    m_searchBtn->setEnabled(loggedIn);
    m_resultsList->setEnabled(loggedIn);
    m_bindBtn->setEnabled(loggedIn);

    TrackingEntry entry = m_tracker->getEntry(m_mangaId);
    bool isBound = (entry.id > 0);

    m_unbindBtn->setVisible(isBound);
    m_statusCombo->setEnabled(loggedIn && isBound);
    m_chaptersSpin->setEnabled(loggedIn && isBound);
    m_scoreSpin->setEnabled(loggedIn && isBound);
    m_updateBtn->setEnabled(loggedIn && isBound);

    if (loggedIn) {
        if (isBound) {
            m_statusLabel->setText(QString("✅ Bound to %1 (ID: %2)").arg(m_tracker->name()).arg(entry.id));
            m_statusLabel->setStyleSheet("color: #A3BE8C; font-weight: bold;");

            int readCh = entry.chaptersRead;
            if (readCh <= 0) {
                // Pre-fill from local reading history if available
                const auto chs = DatabaseManager::instance().chapterRepository().getChaptersByMangaId(m_mangaId);
                for (const auto& c : chs) {
                    if (c.isRead() && c.chapterNumber() > readCh) {
                        readCh = static_cast<int>(c.chapterNumber());
                    }
                }
            }
            m_chaptersSpin->setValue(readCh);
            m_scoreSpin->setValue(entry.score);

            int idx = m_statusCombo->findData(entry.status);
            if (idx >= 0) m_statusCombo->setCurrentIndex(idx);
        } else {
            m_statusLabel->setText(QString("Logged in to %1 (Not bound)").arg(m_tracker->name()));
            m_statusLabel->setStyleSheet("color: #EBCB8B;");
        }
    } else {
        m_statusLabel->setText(QString("Not logged in to %1").arg(m_tracker->name()));
        m_statusLabel->setStyleSheet("color: #BF616A;");
    }
}

void TrackingWidget::onLoginClicked()
{
    if (m_tracker->id() == "mangaupdates") {
        QDialog dialog(this);
        dialog.setWindowTitle("Login to MangaUpdates");
        QFormLayout form(&dialog);

        QLineEdit userEdit(&dialog);
        QLineEdit passEdit(&dialog);
        passEdit.setEchoMode(QLineEdit::Password);

        form.addRow("Username:", &userEdit);
        form.addRow("Password:", &passEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QString user = userEdit.text().trimmed();
            QString pass = passEdit.text();
            if (!user.isEmpty() && !pass.isEmpty()) {
                MangaUpdatesTracker::instance().loginWithCredentials(user, pass);
            }
        }
    } else if (m_tracker->id() == "myanimelist") {
        QDesktopServices::openUrl(m_tracker->authUrl());

        bool ok;
        QString code = QInputDialog::getText(this, "MyAnimeList Login",
            "After approving in browser, paste the authorization 'code' parameter from the URL:",
            QLineEdit::Normal, "", &ok);

        if (ok && !code.trimmed().isEmpty()) {
            m_tracker->login(code.trimmed());
        }
    } else {
        // AniList
        QDesktopServices::openUrl(m_tracker->authUrl());

        bool ok;
        QString token = QInputDialog::getText(this, "AniList Login",
            "After authorizing in browser, paste the access_token parameter from the URL:",
            QLineEdit::Normal, "", &ok);

        if (ok && !token.trimmed().isEmpty()) {
            m_tracker->login(token.trimmed());
        }
    }
}

void TrackingWidget::onLogoutClicked()
{
    m_tracker->logout();
    refreshState();
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

    if (results.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("No results found. Try modifying the search term.", m_resultsList);
        item->setFlags(Qt::NoItemFlags);
        return;
    }

    for (const TrackingSearchResult& r : results) {
        QString text = r.totalChapters > 0
            ? QString("%1 (%2 chapters)").arg(r.title).arg(r.totalChapters)
            : r.title;
        QListWidgetItem *item = new QListWidgetItem(text, m_resultsList);
        item->setData(Qt::UserRole, static_cast<qlonglong>(r.remoteId));
    }
    m_resultsList->setCurrentRow(0);
}

void TrackingWidget::onBindClicked()
{
    QListWidgetItem *item = m_resultsList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Error", "Please select a series from the search results to bind.");
        return;
    }

    long remoteId = item->data(Qt::UserRole).toLongLong();
    m_tracker->bind(m_mangaId, remoteId);
    refreshState();
    QMessageBox::information(this, "Bound", QString("Successfully bound to \"%1\"!\nYou can now adjust status, chapters read, or score and click 'Update Progress'.").arg(item->text()));
}

void TrackingWidget::onUnbindClicked()
{
    m_tracker->unbind(m_mangaId);
    refreshState();
}

void TrackingWidget::onUpdateClicked()
{
    int status = m_statusCombo->currentData().toInt();
    int chapters = m_chaptersSpin->value();
    float score = static_cast<float>(m_scoreSpin->value());

    m_tracker->update(m_mangaId, chapters, status, score);
}
