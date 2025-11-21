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
    QWidget* createPlaceholderPage(const QString& title);

    QVBoxLayout *m_mainLayout;
    QPushButton *m_backButton;
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
};

#endif // SETTINGSVIEW_H
