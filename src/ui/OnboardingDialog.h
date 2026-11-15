#ifndef ONBOARDINGDIALOG_H
#define ONBOARDINGDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

class OnboardingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnboardingDialog(QWidget *parent = nullptr);

signals:
    void onboardingFinished();

private slots:
    void onNextClicked();
    void onBackClicked();
    void onBrowseLocalMangaClicked();

private:
    void setupUi();
    void restyle();
    void applySettings();

    QWidget *createWelcomePage();
    QWidget *createStoragePage();
    QWidget *createAppearancePage();
    QWidget *createExtensionsPage();

    QStackedWidget *m_stack;
    QLabel *m_stepIndicator;
    QPushButton *m_backButton;
    QPushButton *m_skipButton;
    QPushButton *m_nextButton;

    // Storage Page widgets
    QLineEdit *m_localMangaPathEdit;

    // Appearance Page widgets
    QComboBox *m_themeCombo;
    QCheckBox *m_amoledCheck;
    QComboBox *m_readerModeCombo;

    // Extensions Page widgets
    QCheckBox *m_enableCommunityRepoCheck;
};

#endif // ONBOARDINGDIALOG_H
