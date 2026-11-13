#include "ReaderSettingsOverlay.h"
#include "../ThemeManager.h"
#include <QGraphicsDropShadowEffect>

ReaderSettingsOverlay::ReaderSettingsOverlay(QWidget *parent)
    : QFrame(parent)
{
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
}

void ReaderSettingsOverlay::setupUi()
{
    // (No graphics effect: it breaks QWidget::render and offscreen painting)

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // Title
    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Reader Settings", this);
    titleLabel->setObjectName("overlayTitle");

    QPushButton *backBtn = new QPushButton("← Back to Library", this);
    backBtn->setObjectName("overlayBackBtn");
    connect(backBtn, &QPushButton::clicked, this, &ReaderSettingsOverlay::backToLibraryRequested);

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setObjectName("overlayCloseBtn");
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
    m_readingModeCombo->addItem("Double Page", 3);
    m_readingModeCombo->addItem("Double Page (Left to Right)", 4);
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

    // Brightness (applied to the page canvas, like Mihon's custom brightness)
    QLabel *brightnessLabel = new QLabel("Brightness", this);
    m_brightnessSlider = new QSlider(Qt::Horizontal, this);
    m_brightnessSlider->setRange(10, 100);
    m_brightnessSlider->setValue(100);
    connect(m_brightnessSlider, &QSlider::valueChanged, this, &ReaderSettingsOverlay::brightnessChanged);
    layout->addWidget(brightnessLabel);
    layout->addWidget(m_brightnessSlider);

    // Double Page Options
    m_firstPageCoverCheck = new QCheckBox("First page is cover", this);
    connect(m_firstPageCoverCheck, &QCheckBox::toggled, this, &ReaderSettingsOverlay::firstPageCoverChanged);
    layout->addWidget(m_firstPageCoverCheck);

    m_autoWideDetectCheck = new QCheckBox("Don't pair wide pages", this);
    connect(m_autoWideDetectCheck, &QCheckBox::toggled, this, &ReaderSettingsOverlay::autoWideDetectChanged);
    layout->addWidget(m_autoWideDetectCheck);

    layout->addStretch();

    restyle();
}

void ReaderSettingsOverlay::restyle()
{
    const M3Colors& c = ThemeManager::instance().colors();
    setStyleSheet(
        QStringLiteral(
        "ReaderSettingsOverlay {"
        "   background-color: %1;"
        "   border-radius: 16px;"
        "}"
        "QLabel { color: %2; font-weight: bold; background: transparent; }"
        "QCheckBox { color: %2; font-weight: normal; background: transparent; }"
        "#overlayTitle { font-size: 16px; }"
        "#overlayBackBtn { background-color: %3; color: %4; padding: 8px 14px;"
        "  border-radius: 20px; border: none; font-weight: 600; }"
        "#overlayBackBtn:hover { background-color: %5; }"
        "#overlayCloseBtn { background: transparent; border: none; font-size: 16px; color: %2; }"
        "#overlayCloseBtn:hover { color: %6; }")
        .arg(c.surfaceContainerHigh.name(),
             c.onSurface.name(),
             c.secondaryContainer.name(),
             c.onSecondaryContainer.name(),
             ThemeManager::rgba(c.onSurface, 0.12f),
             c.error.name()));
}

void ReaderSettingsOverlay::updateDoublePageControlsVisibility(int mode)
{
    bool isDoublePage = (mode == 3 || mode == 4);
    if (m_firstPageCoverCheck) m_firstPageCoverCheck->setVisible(isDoublePage);
    if (m_autoWideDetectCheck) m_autoWideDetectCheck->setVisible(isDoublePage);
}

void ReaderSettingsOverlay::setCurrentReadingMode(int mode)
{
    int index = m_readingModeCombo->findData(mode);
    if (index >= 0) m_readingModeCombo->setCurrentIndex(index);
    updateDoublePageControlsVisibility(mode);
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

void ReaderSettingsOverlay::setFirstPageCover(bool enabled)
{
    if (m_firstPageCoverCheck) {
        m_firstPageCoverCheck->blockSignals(true);
        m_firstPageCoverCheck->setChecked(enabled);
        m_firstPageCoverCheck->blockSignals(false);
    }
}

void ReaderSettingsOverlay::setAutoWideDetect(bool enabled)
{
    if (m_autoWideDetectCheck) {
        m_autoWideDetectCheck->blockSignals(true);
        m_autoWideDetectCheck->setChecked(enabled);
        m_autoWideDetectCheck->blockSignals(false);
    }
}
