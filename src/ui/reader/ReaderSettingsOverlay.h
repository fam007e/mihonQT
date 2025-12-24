#ifndef READERSETTINGSOVERLAY_H
#define READERSETTINGSOVERLAY_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

class ReaderSettingsOverlay : public QFrame
{
    Q_OBJECT

public:
    explicit ReaderSettingsOverlay(QWidget *parent = nullptr);

    void setCurrentReadingMode(int mode);
    void setCurrentScaleType(int type);
    void setCurrentBrightness(int brightness);

signals:
    void closeRequested();
    void backToLibraryRequested();
    void readingModeChanged(int mode);
    void scaleTypeChanged(int type);
    void brightnessChanged(int brightness);

private:
    void setupUi();

    QComboBox *m_readingModeCombo;
    QComboBox *m_scaleTypeCombo;
    QSlider *m_brightnessSlider;
};

#endif // READERSETTINGSOVERLAY_H
