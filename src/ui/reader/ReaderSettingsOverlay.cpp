#include "ReaderSettingsOverlay.h"
#include <QGraphicsDropShadowEffect>

ReaderSettingsOverlay::ReaderSettingsOverlay(QWidget *parent)
    : QFrame(parent)
{
    setupUi();
}

void ReaderSettingsOverlay::setupUi()
{
    setFixedWidth(300);
    setMinimumHeight(200);
    setStyleSheet(
        "ReaderSettingsOverlay {"
        "   background-color: #2E3440;"
        "   border-radius: 12px;"
        "   border: 1px solid #4C566A;"
        "}"
        "QLabel { color: #ECEFF4; font-weight: bold; }"
        "QComboBox {"
        "   background-color: #3B4252;"
        "   color: #ECEFF4;"
        "   border: 1px solid #4C566A;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "   min-width: 150px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: #3B4252;"
        "   color: #ECEFF4;"
        "   selection-background-color: #88C0D0;"
        "   border: 1px solid #4C566A;"
        "}"
        "QComboBox::drop-down { border: none; }"
        "QSlider::groove:horizontal {"
        "   background: #4C566A;"
        "   height: 8px;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #88C0D0;"
        "   width: 18px;"
        "   margin: -5px 0;"
        "   border-radius: 9px;"
        "}"
    );

    // Shadow effect
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // Title
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Reader Settings", this);
    titleLabel->setStyleSheet("font-size: 16px;");

    QPushButton *backBtn = new QPushButton("← Back to Library", this);
    backBtn->setStyleSheet(
        "QPushButton { background: #4C566A; color: #ECEFF4; padding: 6px 12px; border-radius: 4px; border: none; font-weight: bold; }"
        "QPushButton:hover { background: #5E81AC; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &ReaderSettingsOverlay::backToLibraryRequested);

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setFixedSize(24, 24);
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #ECEFF4; font-size: 16px; border: none; }"
        "QPushButton:hover { color: #BF616A; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &ReaderSettingsOverlay::closeRequested);

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(backBtn);
    titleLayout->addSpacing(10);
    titleLayout->addWidget(closeBtn);
    layout->addLayout(titleLayout);

    // Reading Mode
    QLabel *modeLabel = new QLabel("Reading Mode", this);
    m_readingModeCombo = new QComboBox(this);
    m_readingModeCombo->addItem("Webtoon", 0);
    m_readingModeCombo->addItem("Left to Right", 1);
    m_readingModeCombo->addItem("Right to Left", 2);
    m_readingModeCombo->addItem("Double Page", 3); // Smart Double Page (RTL/LTR based on context)
    connect(m_readingModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        emit readingModeChanged(m_readingModeCombo->itemData(index).toInt());
    });
    layout->addWidget(modeLabel);
    layout->addWidget(m_readingModeCombo);

    // Scale Type
    QLabel *scaleLabel = new QLabel("Scale Type", this);
    m_scaleTypeCombo = new QComboBox(this);
    m_scaleTypeCombo->addItem("Fit Screen", 1);
    m_scaleTypeCombo->addItem("Stretch", 2);
    m_scaleTypeCombo->addItem("Fit Width", 3);
    m_scaleTypeCombo->addItem("Fit Height", 4);
    m_scaleTypeCombo->addItem("Original Size", 5);
    m_scaleTypeCombo->addItem("Smart Fit", 6);
    connect(m_scaleTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        emit scaleTypeChanged(m_scaleTypeCombo->itemData(index).toInt());
    });
    layout->addWidget(scaleLabel);
    layout->addWidget(m_scaleTypeCombo);

    // Brightness
    QLabel *brightnessLabel = new QLabel("Brightness", this);
    m_brightnessSlider = new QSlider(Qt::Horizontal, this);
    m_brightnessSlider->setRange(10, 100);
    m_brightnessSlider->setValue(100);
    connect(m_brightnessSlider, &QSlider::valueChanged, this, &ReaderSettingsOverlay::brightnessChanged);
    layout->addWidget(brightnessLabel);
    layout->addWidget(m_brightnessSlider);

    layout->addStretch();
}

void ReaderSettingsOverlay::setCurrentReadingMode(int mode)
{
    // Map specific Double Page LTR (4) to generic Double Page (3) for UI display
    if (mode == 4) mode = 3;

    int index = m_readingModeCombo->findData(mode);
    if (index >= 0) m_readingModeCombo->setCurrentIndex(index);
}

void ReaderSettingsOverlay::setCurrentScaleType(int type)
{
    int index = m_scaleTypeCombo->findData(type);
    if (index >= 0) m_scaleTypeCombo->setCurrentIndex(index);
}

void ReaderSettingsOverlay::setCurrentBrightness(int brightness)
{
    m_brightnessSlider->setValue(brightness);
}
