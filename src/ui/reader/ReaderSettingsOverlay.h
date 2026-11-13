#ifndef READERSETTINGSOVERLAY_H
#define READERSETTINGSOVERLAY_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>

class ReaderSettingsOverlay : public QFrame
{
    Q_OBJECT

public:
    explicit ReaderSettingsOverlay(QWidget *parent = nullptr);

    void setCurrentReadingMode(int mode);
    void setCurrentScaleType(int type);
    void setCurrentBrightness(int brightness);
    void setFirstPageCover(bool enabled);
    void setAutoWideDetect(bool enabled);

signals:
    void closeRequested();
    void backToLibraryRequested();
    void readingModeChanged(int mode);
    void scaleTypeChanged(int type);
    void brightnessChanged(int brightness);
    void firstPageCoverChanged(bool enabled);
    void autoWideDetectChanged(bool enabled);

private:
    void setupUi();
    void updateDoublePageControlsVisibility(int mode);

    QComboBox *m_readingModeCombo;
    QComboBox *m_scaleTypeCombo;
    QSlider *m_brightnessSlider;
    QCheckBox *m_firstPageCoverCheck;
    QCheckBox *m_autoWideDetectCheck;
};

#endif // READERSETTINGSOVERLAY_H
