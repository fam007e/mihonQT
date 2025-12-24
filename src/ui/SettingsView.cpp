#include "SettingsView.h"
#include "ThemeManager.h"
#include "UiUtils.h"
#include "config/PreferenceManager.h"
#include "CategoryEditDialog.h"
#include "reader/ReaderWidget.h"
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMouseEvent>

SettingsView::SettingsView(QWidget *parent) : QWidget(parent)
{
    qDebug() << "SettingsView constructor start";
    setupUi();
    qDebug() << "SettingsView constructor end";
}

void SettingsView::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // Content Area (Splitter)
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(splitter);

    // Left Sidebar (Categories)
    m_categoryList = new QListWidget(this);
    m_categoryList->setFixedWidth(200);
    UiUtils::applyListWidgetStyle(m_categoryList);
    m_categoryList->addItem("Appearance");
    m_categoryList->addItem("Library");
    m_categoryList->addItem("Reader");
    m_categoryList->addItem("Downloads");
    m_categoryList->addItem("Tracking");
    m_categoryList->addItem("Browse");
    m_categoryList->addItem("Data & Storage");
    m_categoryList->addItem("Security");
    m_categoryList->addItem("Advanced");
    m_categoryList->addItem("About");

    splitter->addWidget(m_categoryList);

    // Right Content (Stacked Widget)
    m_contentStack = new QStackedWidget(this);
    splitter->addWidget(m_contentStack);

    // Create Pages
    createAppearancePage();     // Index 0
    createLibraryPage();        // Index 1
    createReaderPage();         // Index 2
    createDownloadsPage();      // Index 3
    createTrackingPage();       // Index 4
    createBrowsePage();         // Index 5
    createDataStoragePage();    // Index 6
    createSecurityPage();       // Index 7
    createAdvancedPage();       // Index 8
    createAboutPage();          // Index 9

    // Connect Sidebar
    connect(m_categoryList, &QListWidget::currentRowChanged, this, &SettingsView::onCategoryChanged);

    // Select first item
    m_categoryList->setCurrentRow(0);

    setFocusPolicy(Qt::StrongFocus);
}

void SettingsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit backRequested();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void SettingsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit backRequested();
    QWidget::mouseDoubleClickEvent(event);
}

void SettingsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit backRequested();
    } else {
        QWidget::mousePressEvent(event);
    }
}



void SettingsView::createAppearancePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    // Theme Group
    QGroupBox *themeGroup = new QGroupBox("Theme", this);
    QVBoxLayout *themeLayout = new QVBoxLayout(themeGroup);

    QLabel *themeLabel = new QLabel("Application Theme:", this);
    m_themeComboBox = new QComboBox(this);
    // Order must match ThemeManager::Theme enum
    m_themeComboBox->addItem("Nord Dark");
    m_themeComboBox->addItem("Nord Light");
    m_themeComboBox->addItem("Catppuccin Mocha");
    m_themeComboBox->addItem("Catppuccin Frappe");
    m_themeComboBox->addItem("Tokyo Night");
    m_themeComboBox->addItem("Dracula");

    // Set current theme
    ThemeManager::Theme current = ThemeManager::instance().currentTheme();
    m_themeComboBox->setCurrentIndex((int)current);

    connect(m_themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsView::onThemeChanged);

    m_pureBlackCheckBox = new QCheckBox("Pure Black (AMOLED)", this);
    m_pureBlackCheckBox->setChecked(ThemeManager::instance().isAmoledMode());
    connect(m_pureBlackCheckBox, &QCheckBox::toggled, [](bool checked){
        ThemeManager::instance().setAmoledMode(checked);
    });

    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(m_themeComboBox);
    themeLayout->addWidget(m_pureBlackCheckBox);
    layout->addWidget(themeGroup);

    // Display Group
    QGroupBox *displayGroup = new QGroupBox("Display", this);
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);

    QLabel *langLabel = new QLabel("App Language:", this);
    m_appLanguageComboBox = new QComboBox(this);
    m_appLanguageComboBox->addItem("English");
    m_appLanguageComboBox->addItem("System Default");

    QLabel *dateLabel = new QLabel("Date Format:", this);
    m_dateFormatComboBox = new QComboBox(this);
    m_dateFormatComboBox->addItem("MM/dd/yy");
    m_dateFormatComboBox->addItem("dd/MM/yy");
    m_dateFormatComboBox->addItem("yyyy-MM-dd");

    m_relativeTimeCheckBox = new QCheckBox("Relative Time", this);
    m_relativeTimeCheckBox->setChecked(true);

    m_imagesInDescriptionCheckBox = new QCheckBox("Images in Description", this);
    m_imagesInDescriptionCheckBox->setChecked(false);

    displayLayout->addWidget(langLabel);
    displayLayout->addWidget(m_appLanguageComboBox);
    displayLayout->addWidget(dateLabel);
    displayLayout->addWidget(m_dateFormatComboBox);
    displayLayout->addWidget(m_relativeTimeCheckBox);
    displayLayout->addWidget(m_imagesInDescriptionCheckBox);
    layout->addWidget(displayGroup);

    m_contentStack->addWidget(page);
}



void SettingsView::createLibraryPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    // Global Update Group
    QGroupBox *updateGroup = new QGroupBox("Global Update", this);
    QVBoxLayout *updateLayout = new QVBoxLayout(updateGroup);

    QLabel *intervalLabel = new QLabel("Automatic Update Interval:", this);
    m_updateIntervalComboBox = new QComboBox(this);
    m_updateIntervalComboBox->addItem("Manual", 0);
    m_updateIntervalComboBox->addItem("Every 12 hours", 12);
    m_updateIntervalComboBox->addItem("Every 24 hours", 24);
    m_updateIntervalComboBox->addItem("Every 48 hours", 48);
    m_updateIntervalComboBox->addItem("Every 72 hours", 72);
    m_updateIntervalComboBox->addItem("Weekly", 168);

    // Load saved value
    int savedInterval = PreferenceManager::instance().value("library/update_interval", 0).toInt();
    int index = m_updateIntervalComboBox->findData(savedInterval);
    if (index != -1) m_updateIntervalComboBox->setCurrentIndex(index);

    connect(m_updateIntervalComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int value = m_updateIntervalComboBox->itemData(index).toInt();
        PreferenceManager::instance().setValue("library/update_interval", value);
    });

    m_autoUpdateMetadataCheckBox = new QCheckBox("Refresh Metadata", this);
    m_autoUpdateMetadataCheckBox->setChecked(PreferenceManager::instance().value("library/auto_update_metadata", false).toBool());
    connect(m_autoUpdateMetadataCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("library/auto_update_metadata", checked);
    });

    updateLayout->addWidget(intervalLabel);
    updateLayout->addWidget(m_updateIntervalComboBox);
    updateLayout->addWidget(m_autoUpdateMetadataCheckBox);
    layout->addWidget(updateGroup);

    // Categories Group
    QGroupBox *categoryGroup = new QGroupBox("Categories", this);
    QVBoxLayout *categoryLayout = new QVBoxLayout(categoryGroup);

    m_editCategoriesButton = new QPushButton("Edit Categories", this);
    connect(m_editCategoriesButton, &QPushButton::clicked, this, &SettingsView::onEditCategoriesClicked);

    categoryLayout->addWidget(m_editCategoriesButton);
    layout->addWidget(categoryGroup);

    // Behavior Group
    QGroupBox *behaviorGroup = new QGroupBox("Behavior", this);
    QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);

    QLabel *startActionLabel = new QLabel("Swipe Right Action:", this);
    m_swipeStartActionComboBox = new QComboBox(this);
    m_swipeStartActionComboBox->addItem("Disabled", "disabled");
    m_swipeStartActionComboBox->addItem("Toggle Bookmark", "bookmark");
    m_swipeStartActionComboBox->addItem("Toggle Read", "read");
    m_swipeStartActionComboBox->addItem("Download", "download");

    QString savedStartAction = PreferenceManager::instance().value("library/swipe_start_action", "disabled").toString();
    index = m_swipeStartActionComboBox->findData(savedStartAction);
    if (index != -1) m_swipeStartActionComboBox->setCurrentIndex(index);

    connect(m_swipeStartActionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        QString value = m_swipeStartActionComboBox->itemData(index).toString();
        PreferenceManager::instance().setValue("library/swipe_start_action", value);
    });

    QLabel *endActionLabel = new QLabel("Swipe Left Action:", this);
    m_swipeEndActionComboBox = new QComboBox(this);
    m_swipeEndActionComboBox->addItem("Disabled", "disabled");
    m_swipeEndActionComboBox->addItem("Toggle Bookmark", "bookmark");
    m_swipeEndActionComboBox->addItem("Toggle Read", "read");
    m_swipeEndActionComboBox->addItem("Download", "download");

    QString savedEndAction = PreferenceManager::instance().value("library/swipe_end_action", "disabled").toString();
    index = m_swipeEndActionComboBox->findData(savedEndAction);
    if (index != -1) m_swipeEndActionComboBox->setCurrentIndex(index);

    connect(m_swipeEndActionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        QString value = m_swipeEndActionComboBox->itemData(index).toString();
        PreferenceManager::instance().setValue("library/swipe_end_action", value);
    });

    behaviorLayout->addWidget(startActionLabel);
    behaviorLayout->addWidget(m_swipeStartActionComboBox);
    behaviorLayout->addWidget(endActionLabel);
    behaviorLayout->addWidget(m_swipeEndActionComboBox);
    layout->addWidget(behaviorGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createReaderPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    // Reading Mode Group
    QGroupBox *modeGroup = new QGroupBox("Reading Mode", this);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeGroup);

    QLabel *modeLabel = new QLabel("Default Reading Mode:", this);
    m_readingModeComboBox = new QComboBox(this);
    m_readingModeComboBox->addItem("Webtoon", 0);
    m_readingModeComboBox->addItem("Left to Right", 1);
    m_readingModeComboBox->addItem("Right to Left", 2);
    m_readingModeComboBox->addItem("Double Page Spread", 3);

    int savedMode = PreferenceManager::instance().value("reader/default_mode", 0).toInt();
    int index = m_readingModeComboBox->findData(savedMode);
    if (index != -1) m_readingModeComboBox->setCurrentIndex(index);

    connect(m_readingModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int value = m_readingModeComboBox->itemData(index).toInt();
        PreferenceManager::instance().setValue("reader/default_mode", value);
        emit readingModeChanged(value);
        // Force reader to reload settings if active
        // Ideally we'd have a signal for this, but for now we rely on ReaderWidget checking prefs or being re-created
        // Actually, let's emit a generic signal or have ReaderWidget listen to something.
        // For this task, we will just ensure ReaderWidget reloads when shown/created.
        // But for immediate effect, we need a signal.
    });

    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(m_readingModeComboBox);
    layout->addWidget(modeGroup);

    // Display Group
    QGroupBox *displayGroup = new QGroupBox("Display", this);
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);

    m_fullscreenCheckBox = new QCheckBox("Fullscreen", this);
    m_fullscreenCheckBox->setChecked(PreferenceManager::instance().value("reader/fullscreen", false).toBool());
    connect(m_fullscreenCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("reader/fullscreen", checked);
    });

    m_keepScreenOnCheckBox = new QCheckBox("Keep Screen On", this);
    m_keepScreenOnCheckBox->setChecked(PreferenceManager::instance().value("reader/keep_screen_on", false).toBool());
    connect(m_keepScreenOnCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("reader/keep_screen_on", checked);
    });

    m_showPageNumberCheckBox = new QCheckBox("Show Page Number", this);
    m_showPageNumberCheckBox->setChecked(PreferenceManager::instance().value("reader/show_page_number", true).toBool());
    connect(m_showPageNumberCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("reader/show_page_number", checked);
    });

    displayLayout->addWidget(m_fullscreenCheckBox);
    displayLayout->addWidget(m_keepScreenOnCheckBox);
    displayLayout->addWidget(m_showPageNumberCheckBox);
    layout->addWidget(displayGroup);

    // Reading Group
    QGroupBox *readingGroup = new QGroupBox("Reading", this);
    QVBoxLayout *readingLayout = new QVBoxLayout(readingGroup);

    m_pageTransitionsCheckBox = new QCheckBox("Page Transitions", this);
    m_pageTransitionsCheckBox->setChecked(PreferenceManager::instance().value("reader/page_transitions", true).toBool());
    connect(m_pageTransitionsCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("reader/page_transitions", checked);
    });

    QLabel *speedLabel = new QLabel("Double Tap Animation Speed:", this);
    m_doubleTapAnimSpeedComboBox = new QComboBox(this);
    m_doubleTapAnimSpeedComboBox->addItem("No Animation", 1);
    m_doubleTapAnimSpeedComboBox->addItem("Normal", 500);
    m_doubleTapAnimSpeedComboBox->addItem("Fast", 250);

    int savedSpeed = PreferenceManager::instance().value("reader/double_tap_speed", 500).toInt();
    index = m_doubleTapAnimSpeedComboBox->findData(savedSpeed);
    if (index != -1) m_doubleTapAnimSpeedComboBox->setCurrentIndex(index);

    connect(m_doubleTapAnimSpeedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int value = m_doubleTapAnimSpeedComboBox->itemData(index).toInt();
        PreferenceManager::instance().setValue("reader/double_tap_speed", value);
    });

    readingLayout->addWidget(m_pageTransitionsCheckBox);
    readingLayout->addWidget(speedLabel);
    readingLayout->addWidget(m_doubleTapAnimSpeedComboBox);
    layout->addWidget(readingGroup);

    // Paged Mode Group
    QGroupBox *pagedGroup = new QGroupBox("Paged Mode", this);
    QVBoxLayout *pagedLayout = new QVBoxLayout(pagedGroup);

    QLabel *scaleLabel = new QLabel("Scale Type:", this);
    m_scaleTypeComboBox = new QComboBox(this);
    m_scaleTypeComboBox->addItem("Fit Screen", 1);
    m_scaleTypeComboBox->addItem("Stretch", 2);
    m_scaleTypeComboBox->addItem("Fit Width", 3);
    m_scaleTypeComboBox->addItem("Fit Height", 4);
    m_scaleTypeComboBox->addItem("Original Size", 5);
    m_scaleTypeComboBox->addItem("Smart Fit", 6);

    int savedScale = PreferenceManager::instance().value("reader/scale_type", 1).toInt();
    index = m_scaleTypeComboBox->findData(savedScale);
    if (index != -1) m_scaleTypeComboBox->setCurrentIndex(index);

    connect(m_scaleTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int value = m_scaleTypeComboBox->itemData(index).toInt();
        PreferenceManager::instance().setValue("reader/scale_type", value);
    });

    QLabel *zoomLabel = new QLabel("Zoom Start:", this);
    m_zoomStartComboBox = new QComboBox(this);
    m_zoomStartComboBox->addItem("Automatic", 1);
    m_zoomStartComboBox->addItem("Left", 2);
    m_zoomStartComboBox->addItem("Right", 3);
    m_zoomStartComboBox->addItem("Center", 4);

    int savedZoom = PreferenceManager::instance().value("reader/zoom_start", 1).toInt();
    index = m_zoomStartComboBox->findData(savedZoom);
    if (index != -1) m_zoomStartComboBox->setCurrentIndex(index);

    connect(m_zoomStartComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int value = m_zoomStartComboBox->itemData(index).toInt();
        PreferenceManager::instance().setValue("reader/zoom_start", value);
    });

    m_cropBordersCheckBox = new QCheckBox("Crop Borders", this);
    m_cropBordersCheckBox->setChecked(PreferenceManager::instance().value("reader/crop_borders", false).toBool());
    connect(m_cropBordersCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("reader/crop_borders", checked);
    });

    pagedLayout->addWidget(scaleLabel);
    pagedLayout->addWidget(m_scaleTypeComboBox);
    pagedLayout->addWidget(zoomLabel);
    pagedLayout->addWidget(m_zoomStartComboBox);
    pagedLayout->addWidget(m_cropBordersCheckBox);
    layout->addWidget(pagedGroup);

    // Webtoon Mode Group
    QGroupBox *webtoonGroup = new QGroupBox("Webtoon Mode", this);
    QVBoxLayout *webtoonLayout = new QVBoxLayout(webtoonGroup);

    QLabel *paddingLabel = new QLabel("Side Padding:", this);
    m_webtoonSidePaddingComboBox = new QComboBox(this);
    m_webtoonSidePaddingComboBox->addItem("0%", 0);
    m_webtoonSidePaddingComboBox->addItem("5%", 5);
    m_webtoonSidePaddingComboBox->addItem("10%", 10);
    m_webtoonSidePaddingComboBox->addItem("15%", 15);
    m_webtoonSidePaddingComboBox->addItem("20%", 20);
    m_webtoonSidePaddingComboBox->addItem("25%", 25);

    int savedPadding = PreferenceManager::instance().value("reader/webtoon_padding", 0).toInt();
    index = m_webtoonSidePaddingComboBox->findData(savedPadding);
    if (index != -1) m_webtoonSidePaddingComboBox->setCurrentIndex(index);

    connect(m_webtoonSidePaddingComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int value = m_webtoonSidePaddingComboBox->itemData(index).toInt();
        PreferenceManager::instance().setValue("reader/webtoon_padding", value);
    });

    webtoonLayout->addWidget(paddingLabel);
    webtoonLayout->addWidget(m_webtoonSidePaddingComboBox);
    layout->addWidget(webtoonGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createDownloadsPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *downloadGroup = new QGroupBox("Downloads", this);
    QVBoxLayout *downloadLayout = new QVBoxLayout(downloadGroup);

    m_downloadNewChaptersCheckBox = new QCheckBox("Download new chapters", this);
    m_downloadNewChaptersCheckBox->setChecked(PreferenceManager::instance().value("downloads/download_new", false).toBool());
    connect(m_downloadNewChaptersCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("downloads/download_new", checked);
    });

    m_deleteRemovedChaptersCheckBox = new QCheckBox("Delete chapters after reading", this);
    m_deleteRemovedChaptersCheckBox->setChecked(PreferenceManager::instance().value("downloads/delete_removed", false).toBool());
    connect(m_deleteRemovedChaptersCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("downloads/delete_removed", checked);
    });


    downloadLayout->addWidget(m_downloadNewChaptersCheckBox);
    downloadLayout->addWidget(m_deleteRemovedChaptersCheckBox);
    layout->addWidget(downloadGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createTrackingPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *trackingGroup = new QGroupBox("Tracking", this);
    QVBoxLayout *trackingLayout = new QVBoxLayout(trackingGroup);

    m_autoUpdateTrackersCheckBox = new QCheckBox("Automatically update trackers", this);
    m_autoUpdateTrackersCheckBox->setChecked(PreferenceManager::instance().value("tracking/auto_update", true).toBool());
    connect(m_autoUpdateTrackersCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("tracking/auto_update", checked);
    });

    m_loginMalButton = new QPushButton("Login to MyAnimeList", this);
    m_loginAnilistButton = new QPushButton("Login to AniList", this);

    trackingLayout->addWidget(m_autoUpdateTrackersCheckBox);
    trackingLayout->addWidget(m_loginMalButton);
    trackingLayout->addWidget(m_loginAnilistButton);
    layout->addWidget(trackingGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createBrowsePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *browseGroup = new QGroupBox("Browse", this);
    QVBoxLayout *browseLayout = new QVBoxLayout(browseGroup);

    m_checkForSourceUpdatesCheckBox = new QCheckBox("Check for source updates", this);
    m_checkForSourceUpdatesCheckBox->setChecked(PreferenceManager::instance().value("browse/check_updates", true).toBool());
    connect(m_checkForSourceUpdatesCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("browse/check_updates", checked);
    });

    m_autoUpdateExtensionsCheckBox = new QCheckBox("Auto-update extensions", this);
    m_autoUpdateExtensionsCheckBox->setChecked(PreferenceManager::instance().value("browse/auto_update_extensions", false).toBool());
    connect(m_autoUpdateExtensionsCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("browse/auto_update_extensions", checked);
    });

    m_showNsfwSourceCheckBox = new QCheckBox("Show NSFW sources", this);
    m_showNsfwSourceCheckBox->setChecked(PreferenceManager::instance().value("browse/show_nsfw", true).toBool());
    connect(m_showNsfwSourceCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("browse/show_nsfw", checked);
    });

    m_localSourceLocationButton = new QPushButton("Local Source Folder", this);
    connect(m_localSourceLocationButton, &QPushButton::clicked, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, "Select Local Manga Directory",
                                                      QDir::homePath(),
                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            QSettings settings("MihonQT", "MihonQT");
            settings.setValue("localMangaPath", dir);
            emit localMangaPathChanged(dir);
            QMessageBox::information(this, "Success", "Local manga path updated to:\n" + dir);
        }
    });

    browseLayout->addWidget(m_checkForSourceUpdatesCheckBox);
    browseLayout->addWidget(m_autoUpdateExtensionsCheckBox);
    browseLayout->addWidget(m_showNsfwSourceCheckBox);
    browseLayout->addWidget(m_localSourceLocationButton);
    layout->addWidget(browseGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createDataStoragePage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *storageGroup = new QGroupBox("Storage", this);
    QVBoxLayout *storageLayout = new QVBoxLayout(storageGroup);

    m_createBackupButton = new QPushButton("Create Backup", this);
    m_restoreBackupButton = new QPushButton("Restore Backup", this);
    m_clearCacheButton = new QPushButton("Clear Chapter Cache", this);
    m_clearCookiesButton = new QPushButton("Clear Cookie Jar", this);

    storageLayout->addWidget(m_createBackupButton);
    storageLayout->addWidget(m_restoreBackupButton);
    storageLayout->addWidget(m_clearCacheButton);
    storageLayout->addWidget(m_clearCookiesButton);
    layout->addWidget(storageGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createSecurityPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *securityGroup = new QGroupBox("Security", this);
    QVBoxLayout *securityLayout = new QVBoxLayout(securityGroup);

    QCheckBox *incognitoCheckBox = new QCheckBox("Incognito Mode", this);
    incognitoCheckBox->setChecked(PreferenceManager::instance().value("security/incognito", false).toBool());
    connect(incognitoCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("security/incognito", checked);
    });

    QCheckBox *secureScreenCheckBox = new QCheckBox("Secure Screen", this);
    secureScreenCheckBox->setChecked(PreferenceManager::instance().value("security/secure_screen", false).toBool());
    connect(secureScreenCheckBox, &QCheckBox::toggled, [](bool checked){
        PreferenceManager::instance().setValue("security/secure_screen", checked);
    });

    securityLayout->addWidget(incognitoCheckBox);
    securityLayout->addWidget(secureScreenCheckBox);
    layout->addWidget(securityGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createAdvancedPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *advancedGroup = new QGroupBox("Advanced", this);
    QVBoxLayout *advancedLayout = new QVBoxLayout(advancedGroup);

    QPushButton *clearDbButton = new QPushButton("Clear Database", this);
    QPushButton *dumpLogsButton = new QPushButton("Dump Crash Logs", this);

    advancedLayout->addWidget(clearDbButton);
    advancedLayout->addWidget(dumpLogsButton);
    layout->addWidget(advancedGroup);

    m_contentStack->addWidget(page);
}

void SettingsView::createAboutPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    // App Logo Placeholder (using a stylized M)
    QLabel *logoLabel = new QLabel(this);
    logoLabel->setText("M");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedSize(120, 120);
    logoLabel->setStyleSheet(
        "background-color: #88C0D0;"
        "color: #2E3440;"
        "border-radius: 60px;"
        "font-size: 72px;"
        "font-weight: bold;"
    );
    layout->addWidget(logoLabel);

    // Info
    QLabel *titleLabel = new QLabel("MihonQT", this);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #ECEFF4;");
    layout->addWidget(titleLabel);

    QLabel *versionLabel = new QLabel("Version 0.1.0-alpha", this);
    versionLabel->setStyleSheet("font-size: 14px; color: #D8DEE9;");
    layout->addWidget(versionLabel);

    QLabel *descLabel = new QLabel(
        "A powerful, cross-platform manga reader for desktop.<br>"
        "Inspired by the beloved Mihon and Tachiyomi projects.<br>"
        "<br>"
        "Built with C++, Qt 6, and ❤️.",
        this
    );
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("font-size: 14px; color: #ECEFF4; margin: 20px;");
    layout->addWidget(descLabel);

    // Links
    QHBoxLayout *linksLayout = new QHBoxLayout();
    linksLayout->setAlignment(Qt::AlignCenter);
    linksLayout->setSpacing(15);

    auto createLink = [this](const QString& text, const QString& url) {
        QPushButton *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { "
            "   background-color: #4C566A; "
            "   color: #ECEFF4; "
            "   border: none; "
            "   padding: 8px 16px; "
            "   border-radius: 4px; "
            "}"
            "QPushButton:hover { background-color: #5E81AC; }"
        );
        // In a real app, use QDesktopServices::openUrl
        return btn;
    };

    linksLayout->addWidget(createLink("GitHub", "https://github.com/fam007e/mihonQT"));
    linksLayout->addWidget(createLink("Website", "https://mihon.app"));
    linksLayout->addWidget(createLink("Discord", "https://discord.gg/mihon"));
    layout->addLayout(linksLayout);

    layout->addStretch();

    QLabel *footerLabel = new QLabel("© 2025 MihonQT Contributors. Licensed under AGPLv3.", this);
    footerLabel->setStyleSheet("font-size: 11px; color: #4C566A; padding-bottom: 20px;");
    layout->addWidget(footerLabel);

    m_contentStack->addWidget(page);
}

QWidget* SettingsView::createPlaceholderPage(const QString& title)
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *label = new QLabel(title, this);
    label->setStyleSheet("font-size: 24px; color: palette(text);");
    QLabel *subLabel = new QLabel("Not implemented yet", this);
    subLabel->setStyleSheet("font-size: 14px; color: palette(mid);");

    layout->addWidget(label);
    layout->addWidget(subLabel);

    return page;
}

void SettingsView::onCategoryChanged(int index)
{
    // Map list index to stack index
    // In our case they map 1:1 because we added them in order
    // But we need to handle the fact that createAppearancePage etc add to stack internally
    // Let's verify the order:
    // 0: Appearance (created by createAppearancePage)
    // 1: Library (placeholder)
    // 2: Reader (created by createReaderPage)
    // ...
    // Yes, the order of addWidget calls in setupUi determines the index.
    // We added them in the exact same order as the list items.

    if (index >= 0 && index < m_contentStack->count()) {
        m_contentStack->setCurrentIndex(index);
    }
}

void SettingsView::onThemeChanged(int index)
{
    ThemeManager::Theme theme;
    switch (index) {
    case 0: theme = ThemeManager::NordDark; break;
    case 1: theme = ThemeManager::NordLight; break;
    case 2: theme = ThemeManager::CatppuccinMocha; break;
    case 3: theme = ThemeManager::CatppuccinFrappe; break;
    case 4: theme = ThemeManager::TokyoNight; break;
    case 5: theme = ThemeManager::Dracula; break;
    default: theme = ThemeManager::NordDark; break;
    }
    ThemeManager::instance().applyTheme(theme);
}

void SettingsView::onReadingModeChanged(int index)
{
    emit readingModeChanged(index);
}

void SettingsView::onEditCategoriesClicked()
{
    CategoryEditDialog dialog(this);
    dialog.exec();
}
