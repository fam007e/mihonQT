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
    setFixedWidth(380);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);
    layout->setSizeConstraint(QLayout::SetFixedSize);

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
    titleLayout->addSpacing(8);
    titleLayout->addWidget(closeBtn);
    layout->addLayout(titleLayout);

    // Reading Mode
    QLabel *modeLabel = new QLabel("Reading Mode", this);
    m_readingModeCombo = new QComboBox(this);
    m_readingModeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_readingModeCombo->setFixedHeight(36);
    m_readingModeCombo->addItem("Webtoon", 0);
    m_readingModeCombo->addItem("Left to Right", 1);
    m_readingModeCombo->addItem("Right to Left", 2);
    m_readingModeCombo->addItem("Double Page", 3);
    m_readingModeCombo->addItem("Double Page (Left to Right)", 4);
    connect(m_readingModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        int mode = m_readingModeCombo->itemData(index).toInt();
        updateDoublePageControlsVisibility(mode);
        emit readingModeChanged(mode);
    });
    layout->addWidget(modeLabel);
    layout->addWidget(m_readingModeCombo);

    // Scale Type
    QLabel *scaleLabel = new QLabel("Scale Type", this);
    m_scaleTypeCombo = new QComboBox(this);
    m_scaleTypeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_scaleTypeCombo->setFixedHeight(36);
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
    m_brightnessSlider->setFixedHeight(24);
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

    restyle();
}

void ReaderSettingsOverlay::restyle()
{
    const M3Colors& c = ThemeManager::instance().colors();
    setStyleSheet(
        QStringLiteral(
        "ReaderSettingsOverlay {"
        "   background-color: %1;"
        "   border: 1px solid %2;"
        "   border-radius: 16px;"
        "}"
        "QLabel { color: %3; font-weight: bold; background: transparent; font-size: 13px; }"
        "QCheckBox { color: %3; font-weight: normal; background: transparent; font-size: 13px; spacing: 8px; }"
        "#overlayTitle { font-size: 16px; font-weight: bold; color: %3; }"
        "#overlayBackBtn { background-color: %4; color: %5; padding: 6px 14px;"
        "  border-radius: 16px; border: none; font-weight: 600; font-size: 12px; }"
        "#overlayBackBtn:hover { background-color: %6; }"
        "#overlayCloseBtn { background: transparent; border: none; font-size: 16px; font-weight: bold; color: %3; padding: 2px 6px; }"
        "#overlayCloseBtn:hover { color: %7; }"
        "QComboBox {"
        "   background-color: %8;"
        "   color: %3;"
        "   border: 1px solid %2;"
        "   border-radius: 8px;"
        "   padding: 6px 12px;"
        "   font-size: 13px;"
        "   font-weight: 500;"
        "}"
        "QComboBox:hover { background-color: %9;"
        "   border: 1px solid %10; }"
        "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 24px; border: none; }"
        "QComboBox QAbstractItemView {"
        "   background-color: %1;"
        "   color: %3;"
        "   border: 1px solid %2;"
        "   border-radius: 8px;"
        "   padding: 4px;"
        "   selection-background-color: %4;"
        "   selection-color: %5;"
        "   outline: none;"
        "}"
        "QComboBox QAbstractItemView::item { min-height: 28px; padding: 4px 8px; }"
        "QSlider::groove:horizontal {"
        "   height: 6px;"
        "   background: %2;"
        "   border-radius: 3px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "   background: %10;"
        "   border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: %10;"
        "   width: 16px;"
        "   height: 16px;"
        "   margin: -5px 0;"
        "   border-radius: 8px;"
        "}")
        .arg(c.surfaceContainerHigh.name(),              // %1
             ThemeManager::rgba(c.outlineVariant, 0.4f),  // %2
             c.onSurface.name(),                         // %3
             c.secondaryContainer.name(),                // %4
             c.onSecondaryContainer.name(),              // %5
             ThemeManager::rgba(c.onSurface, 0.12f),     // %6
             c.error.name(),                             // %7
             c.surfaceContainerLow.name(),               // %8
             c.surfaceContainerHighest.name(),           // %9
             c.primary.name()));                         // %10
}

void ReaderSettingsOverlay::updateDoublePageControlsVisibility(int mode)
{
    bool isDoublePage = (mode == 3 || mode == 4);
    if (m_firstPageCoverCheck) m_firstPageCoverCheck->setVisible(isDoublePage);
    if (m_autoWideDetectCheck) m_autoWideDetectCheck->setVisible(isDoublePage);
    adjustSize();
    if (parentWidget()) {
        int x = (parentWidget()->width() - width()) / 2;
        int y = (parentWidget()->height() - height()) / 2;
        move(x, y);
    }
}

void ReaderSettingsOverlay::setCurrentReadingMode(int mode)
{
    m_readingModeCombo->blockSignals(true);
    int index = m_readingModeCombo->findData(mode);
    if (index >= 0) m_readingModeCombo->setCurrentIndex(index);
    m_readingModeCombo->blockSignals(false);
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
