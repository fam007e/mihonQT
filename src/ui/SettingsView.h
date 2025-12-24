#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QStackedWidget>

class SettingsView : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsView(QWidget *parent = nullptr);

signals:
    void backRequested();
    void readingModeChanged(int mode);
    void localMangaPathChanged(const QString& newPath);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onThemeChanged(int index);
    void onReadingModeChanged(int index);
    void onCategoryChanged(int index);
    void onEditCategoriesClicked();

private:
    void setupUi();
    void createAppearancePage();
    void createLibraryPage();
    void createReaderPage();
    void createDownloadsPage();
    void createTrackingPage();
    void createBrowsePage();
    void createDataStoragePage();
    void createSecurityPage();
    void createAdvancedPage();
    void createAboutPage();
    QWidget* createPlaceholderPage(const QString& title);

    QVBoxLayout *m_mainLayout;
    QListWidget *m_categoryList;
    QStackedWidget *m_contentStack;

    // Appearance widgets
    QComboBox *m_themeComboBox;
    QCheckBox *m_pureBlackCheckBox;

    // Display widgets
    QComboBox *m_appLanguageComboBox;
    QComboBox *m_dateFormatComboBox;
    QCheckBox *m_relativeTimeCheckBox;
    QCheckBox *m_imagesInDescriptionCheckBox;

    // Library widgets
    QComboBox *m_updateIntervalComboBox;
    QCheckBox *m_autoUpdateMetadataCheckBox;
    QComboBox *m_swipeStartActionComboBox;
    QComboBox *m_swipeEndActionComboBox;
    QPushButton *m_editCategoriesButton;

    // Reader widgets
    QComboBox *m_readingModeComboBox;
    QComboBox *m_doubleTapAnimSpeedComboBox;
    QCheckBox *m_fullscreenCheckBox;
    QCheckBox *m_keepScreenOnCheckBox;
    QCheckBox *m_showPageNumberCheckBox;
    QCheckBox *m_pageTransitionsCheckBox;

    // Paged Mode widgets
    QComboBox *m_scaleTypeComboBox;
    QComboBox *m_zoomStartComboBox;
    QCheckBox *m_cropBordersCheckBox;

    // Webtoon Mode widgets
    QComboBox *m_webtoonSidePaddingComboBox;

    // Downloads widgets
    QCheckBox *m_downloadNewChaptersCheckBox;
    QCheckBox *m_deleteRemovedChaptersCheckBox;

    // Tracking widgets
    QCheckBox *m_autoUpdateTrackersCheckBox;
    QPushButton *m_loginMalButton;
    QPushButton *m_loginAnilistButton;

    // Browse widgets
    QCheckBox *m_checkForSourceUpdatesCheckBox;
    QCheckBox *m_autoUpdateExtensionsCheckBox;
    QCheckBox *m_showNsfwSourceCheckBox;
    QPushButton *m_localSourceLocationButton;

    // Data & Storage widgets
    QPushButton *m_createBackupButton;
    QPushButton *m_restoreBackupButton;
    QPushButton *m_clearCacheButton;
    QPushButton *m_clearCookiesButton;
};

#endif // SETTINGSVIEW_H
