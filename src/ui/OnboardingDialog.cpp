#include "OnboardingDialog.h"
#include "ThemeManager.h"
#include "../config/PreferenceManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

OnboardingDialog::OnboardingDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Welcome to MihonQT");
    setModal(true);
    resize(560, 500);
    setMinimumSize(480, 440);

    setupUi();
    restyle();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
}

void OnboardingDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    // Top Header with Step indicator
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_stepIndicator = new QLabel("Step 1 of 4", this);
    m_stepIndicator->setObjectName("onboardingStepIndicator");
    topLayout->addWidget(m_stepIndicator);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // Center Stack
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createWelcomePage());
    m_stack->addWidget(createStoragePage());
    m_stack->addWidget(createAppearancePage());
    m_stack->addWidget(createExtensionsPage());
    mainLayout->addWidget(m_stack, 1);

    // Bottom Navigation Bar
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setSpacing(12);

    m_backButton = new QPushButton("Back", this);
    m_backButton->setObjectName("onboardingSecondaryBtn");
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setVisible(false);
    connect(m_backButton, &QPushButton::clicked, this, &OnboardingDialog::onBackClicked);

    m_skipButton = new QPushButton("Skip", this);
    m_skipButton->setObjectName("onboardingTextBtn");
    m_skipButton->setCursor(Qt::PointingHandCursor);
    connect(m_skipButton, &QPushButton::clicked, this, [this]() {
        applySettings();
        accept();
    });

    m_nextButton = new QPushButton("Next →", this);
    m_nextButton->setObjectName("onboardingPrimaryBtn");
    m_nextButton->setCursor(Qt::PointingHandCursor);
    m_nextButton->setMinimumSize(120, 40);
    connect(m_nextButton, &QPushButton::clicked, this, &OnboardingDialog::onNextClicked);

    navLayout->addWidget(m_backButton);
    navLayout->addWidget(m_skipButton);
    navLayout->addStretch();
    navLayout->addWidget(m_nextButton);

    mainLayout->addLayout(navLayout);
}

QWidget *OnboardingDialog::createWelcomePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(14);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *logoLabel = new QLabel(this);
    QPixmap logo(":/icons/book.svg");
    if (!logo.isNull()) {
        logoLabel->setPixmap(logo.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(logoLabel);

    QLabel *title = new QLabel("Welcome to MihonQT", this);
    title->setObjectName("onboardingTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel *subtitle = new QLabel("A native desktop manga reader built with C++ and Qt 6.", this);
    subtitle->setObjectName("onboardingSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    layout->addSpacing(10);

    // Pillar highlights
    auto addPillar = [layout, this](const QString &icon, const QString &heading, const QString &desc) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(12);

        QLabel *iconLbl = new QLabel(icon, this);
        iconLbl->setStyleSheet("font-size: 22px;");
        iconLbl->setFixedWidth(30);
        iconLbl->setAlignment(Qt::AlignCenter);

        QVBoxLayout *textCol = new QVBoxLayout();
        textCol->setSpacing(2);
        QLabel *h = new QLabel(heading, this);
        h->setStyleSheet("font-weight: bold; font-size: 13px; color: palette(text);");
        QLabel *d = new QLabel(desc, this);
        d->setStyleSheet("font-size: 12px; color: palette(mid);");
        d->setWordWrap(true);
        textCol->addWidget(h);
        textCol->addWidget(d);

        row->addWidget(iconLbl);
        row->addLayout(textCol, 1);
        layout->addLayout(row);
    };

    addPillar("🛡️", "Privacy First", "100% local. Zero telemetry, zero analytics, no required accounts.");
    addPillar("⚡", "High Performance", "Built with modern C++17, multi-threaded downloads, and hardware-accelerated rendering.");
    addPillar("⌨️", "Keyboard Optimized", "Navigate seamlessly with arrow keys, WASD, and customizable reader hotkeys.");

    layout->addStretch();
    return page;
}

QWidget *OnboardingDialog::createStoragePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(14);
    layout->setAlignment(Qt::AlignTop);

    QLabel *title = new QLabel("Storage & Local Manga", this);
    title->setObjectName("onboardingTitle");
    layout->addWidget(title);

    QLabel *desc = new QLabel(
        "Choose the default folder where MihonQT discovers local manga (.cbz files, image folders) "
        "and where downloaded chapters are saved for offline reading.", this);
    desc->setObjectName("onboardingSubtitle");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    layout->addSpacing(10);

    QLabel *pathLabel = new QLabel("Local Manga Directory:", this);
    pathLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(pathLabel);

    QHBoxLayout *pathRow = new QHBoxLayout();
    pathRow->setSpacing(8);

    QSettings settings("MihonQT", "MihonQT");
    QString defaultPath = settings.value("localMangaPath", QDir::homePath() + "/Manga").toString();

    m_localMangaPathEdit = new QLineEdit(defaultPath, this);
    m_localMangaPathEdit->setFixedHeight(36);
    m_localMangaPathEdit->setStyleSheet("padding: 0 10px; border-radius: 8px;");

    QPushButton *browseBtn = new QPushButton("Browse...", this);
    browseBtn->setFixedHeight(36);
    browseBtn->setCursor(Qt::PointingHandCursor);
    connect(browseBtn, &QPushButton::clicked, this, &OnboardingDialog::onBrowseLocalMangaClicked);

    pathRow->addWidget(m_localMangaPathEdit, 1);
    pathRow->addWidget(browseBtn);
    layout->addLayout(pathRow);

    QLabel *hintLabel = new QLabel("Tip: You can change this or add more locations anytime in Settings > Browse.", this);
    hintLabel->setStyleSheet("font-size: 11px; color: palette(mid); font-style: italic;");
    hintLabel->setWordWrap(true);
    layout->addWidget(hintLabel);

    layout->addStretch();
    return page;
}

QWidget *OnboardingDialog::createAppearancePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(14);
    layout->setAlignment(Qt::AlignTop);

    QLabel *title = new QLabel("Appearance & Reader", this);
    title->setObjectName("onboardingTitle");
    layout->addWidget(title);

    QLabel *desc = new QLabel("Customize your reading environment. All settings can be adjusted later in Settings.", this);
    desc->setObjectName("onboardingSubtitle");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    layout->addSpacing(10);

    // Theme selector
    QLabel *themeLabel = new QLabel("Color Palette:", this);
    themeLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(themeLabel);

    m_themeCombo = new QComboBox(this);
    m_themeCombo->setFixedHeight(36);
    m_themeCombo->addItem("Mihon Default (Material 3)", (int)ThemeManager::MihonDefault);
    m_themeCombo->addItem("Nord Dark", (int)ThemeManager::NordDark);
    m_themeCombo->addItem("Nord Light", (int)ThemeManager::NordLight);
    m_themeCombo->addItem("Catppuccin Mocha", (int)ThemeManager::CatppuccinMocha);
    m_themeCombo->addItem("Catppuccin Frappe", (int)ThemeManager::CatppuccinFrappe);
    m_themeCombo->addItem("Tokyo Night", (int)ThemeManager::TokyoNight);
    m_themeCombo->addItem("Dracula", (int)ThemeManager::Dracula);

    ThemeManager::Theme currentTheme = ThemeManager::instance().currentTheme();
    int currentIdx = m_themeCombo->findData((int)currentTheme);
    if (currentIdx != -1) m_themeCombo->setCurrentIndex(currentIdx);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        int themeEnum = m_themeCombo->itemData(index).toInt();
        ThemeManager::instance().applyTheme(static_cast<ThemeManager::Theme>(themeEnum));
    });
    layout->addWidget(m_themeCombo);

    // AMOLED mode
    m_amoledCheck = new QCheckBox("Pure Black (AMOLED) Background", this);
    m_amoledCheck->setChecked(ThemeManager::instance().isAmoledMode());
    connect(m_amoledCheck, &QCheckBox::toggled, this, [](bool checked) {
        ThemeManager::instance().setAmoledMode(checked);
    });
    layout->addWidget(m_amoledCheck);

    layout->addSpacing(10);

    // Default Reading Mode
    QLabel *modeLabel = new QLabel("Default Reading Mode:", this);
    modeLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(modeLabel);

    m_readerModeCombo = new QComboBox(this);
    m_readerModeCombo->setFixedHeight(36);
    m_readerModeCombo->addItem("Webtoon (Continuous Vertical)", 0);
    m_readerModeCombo->addItem("Left to Right (Paged)", 1);
    m_readerModeCombo->addItem("Right to Left (Manga Standard)", 2);
    m_readerModeCombo->addItem("Double Page Spread", 3);

    int savedMode = PreferenceManager::instance().value("reader/default_mode", 1).toInt();
    m_readerModeCombo->setCurrentIndex(savedMode);
    layout->addWidget(m_readerModeCombo);

    layout->addStretch();
    return page;
}

QWidget *OnboardingDialog::createExtensionsPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(14);
    layout->setAlignment(Qt::AlignTop);

    QLabel *title = new QLabel("Online Sources & Extensions", this);
    title->setObjectName("onboardingTitle");
    layout->addWidget(title);

    QLabel *desc = new QLabel(
        "MihonQT uses lightweight JavaScript extensions to browse and read manga from community sources. "
        "Sources are sandboxed by default for security.", this);
    desc->setObjectName("onboardingSubtitle");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    layout->addSpacing(10);

    m_enableCommunityRepoCheck = new QCheckBox("Enable Official Community Extensions Repository", this);
    m_enableCommunityRepoCheck->setChecked(true);
    m_enableCommunityRepoCheck->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(m_enableCommunityRepoCheck);

    QLabel *repoDesc = new QLabel(
        "Provides ready-to-install sources (WeebCentral, Toonily, ManhuaUS, HiperDex, Webtoons.com, MangaDex) "
        "with automatic index fetching in More > Extensions.", this);
    repoDesc->setStyleSheet("font-size: 12px; color: palette(mid); margin-left: 24px;");
    repoDesc->setWordWrap(true);
    layout->addWidget(repoDesc);

    layout->addSpacing(16);

    QLabel *readyLabel = new QLabel("🎉 You're all set! Click \"Finish Setup\" to start reading.", this);
    readyLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    readyLabel->setWordWrap(true);
    layout->addWidget(readyLabel);

    layout->addStretch();
    return page;
}

void OnboardingDialog::onBrowseLocalMangaClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Local Manga Directory", m_localMangaPathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_localMangaPathEdit->setText(dir);
    }
}

void OnboardingDialog::onNextClicked()
{
    int currentIndex = m_stack->currentIndex();
    if (currentIndex < m_stack->count() - 1) {
        m_stack->setCurrentIndex(currentIndex + 1);
        m_backButton->setVisible(true);
        m_stepIndicator->setText(QString("Step %1 of %2").arg(currentIndex + 2).arg(m_stack->count()));

        if (currentIndex + 1 == m_stack->count() - 1) {
            m_nextButton->setText("Finish Setup 🎉");
        }
    } else {
        applySettings();
        accept();
    }
}

void OnboardingDialog::onBackClicked()
{
    int currentIndex = m_stack->currentIndex();
    if (currentIndex > 0) {
        m_stack->setCurrentIndex(currentIndex - 1);
        m_stepIndicator->setText(QString("Step %1 of %2").arg(currentIndex).arg(m_stack->count()));
        m_nextButton->setText("Next →");
        if (currentIndex - 1 == 0) {
            m_backButton->setVisible(false);
        }
    }
}

void OnboardingDialog::applySettings()
{
    // Save Local Manga Path
    if (m_localMangaPathEdit) {
        QString path = m_localMangaPathEdit->text().trimmed();
        if (!path.isEmpty()) {
            QDir().mkpath(path);
            QSettings settings("MihonQT", "MihonQT");
            settings.setValue("localMangaPath", path);
        }
    }

    // Save Theme & Reading Mode
    if (m_themeCombo) {
        int themeEnum = m_themeCombo->currentData().toInt();
        PreferenceManager::instance().setValue("theme/current", themeEnum);
        ThemeManager::instance().applyTheme(static_cast<ThemeManager::Theme>(themeEnum));
    }
    if (m_amoledCheck) {
        bool amoled = m_amoledCheck->isChecked();
        PreferenceManager::instance().setValue("theme/amoled", amoled);
        ThemeManager::instance().setAmoledMode(amoled);
    }
    if (m_readerModeCombo) {
        int mode = m_readerModeCombo->currentIndex();
        PreferenceManager::instance().setValue("reader/default_mode", mode);
    }

    emit onboardingFinished();
}

void OnboardingDialog::restyle()
{
    const M3Colors &c = ThemeManager::instance().colors();

    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background-color: %1;"
        "}"
        "#onboardingStepIndicator {"
        "  color: %2;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "  background: transparent;"
        "}"
        "#onboardingTitle {"
        "  font-size: 22px;"
        "  font-weight: bold;"
        "  color: %3;"
        "  background: transparent;"
        "}"
        "#onboardingSubtitle {"
        "  font-size: 13px;"
        "  color: %2;"
        "  background: transparent;"
        "}"
        "#onboardingPrimaryBtn {"
        "  background-color: %4;"
        "  color: %5;"
        "  border: none;"
        "  border-radius: 20px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "  padding: 8px 20px;"
        "}"
        "#onboardingPrimaryBtn:hover {"
        "  background-color: %6;"
        "}"
        "#onboardingSecondaryBtn {"
        "  background-color: %7;"
        "  color: %8;"
        "  border: none;"
        "  border-radius: 20px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "  padding: 8px 18px;"
        "}"
        "#onboardingSecondaryBtn:hover {"
        "  background-color: %9;"
        "}"
        "#onboardingTextBtn {"
        "  background: transparent;"
        "  color: %2;"
        "  border: none;"
        "  font-size: 13px;"
        "  padding: 8px 12px;"
        "}"
        "#onboardingTextBtn:hover {"
        "  color: %3;"
        "}")
        .arg(c.surface.name(),                         // %1
             c.onSurfaceVariant.name(),                // %2
             c.onSurface.name(),                       // %3
             c.primary.name(),                         // %4
             c.onPrimary.name(),                       // %5
             ThemeManager::rgba(c.primary, 0.85f),     // %6
             c.secondaryContainer.name(),              // %7
             c.onSecondaryContainer.name(),            // %8
             ThemeManager::rgba(c.onSurface, 0.12f))); // %9
}
