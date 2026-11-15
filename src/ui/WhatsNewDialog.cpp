#include "WhatsNewDialog.h"
#include "ThemeManager.h"
#include "../AppVersion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

WhatsNewDialog::WhatsNewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("What's New");
    setModal(true);
    resize(520, 520);
    setMinimumSize(420, 420);

    setupUi();
    restyle();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
}

void WhatsNewDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Header: Title + Version Badge
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    m_titleLabel = new QLabel("What's New in MihonQT", this);
    m_titleLabel->setObjectName("whatsNewTitle");

    m_versionBadge = new QLabel(QStringLiteral("v%1").arg(AppVersion::VERSION), this);
    m_versionBadge->setObjectName("whatsNewBadge");

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_versionBadge);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Content: Markdown changelog in QTextBrowser
    m_textBrowser = new QTextBrowser(this);
    m_textBrowser->setFrameShape(QFrame::NoFrame);
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->setMarkdown(AppVersion::getChangelog());
    mainLayout->addWidget(m_textBrowser, 1);

    // Bottom Action Row
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    m_closeButton = new QPushButton("Got it!", this);
    m_closeButton->setObjectName("whatsNewCloseBtn");
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setMinimumSize(120, 40);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(m_closeButton);

    mainLayout->addLayout(bottomLayout);
}

void WhatsNewDialog::restyle()
{
    const M3Colors &c = ThemeManager::instance().colors();

    setStyleSheet(QStringLiteral(
        "QDialog {"
        "  background-color: %1;"
        "}"
        "#whatsNewTitle {"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "  color: %2;"
        "  background: transparent;"
        "}"
        "#whatsNewBadge {"
        "  background-color: %3;"
        "  color: %4;"
        "  border-radius: 10px;"
        "  padding: 3px 8px;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "}"
        "QTextBrowser {"
        "  background-color: %5;"
        "  color: %2;"
        "  border: 1px solid %6;"
        "  border-radius: 12px;"
        "  padding: 12px 16px;"
        "  font-size: 13px;"
        "}"
        "#whatsNewCloseBtn {"
        "  background-color: %7;"
        "  color: %8;"
        "  border: none;"
        "  border-radius: 20px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "  padding: 8px 24px;"
        "}"
        "#whatsNewCloseBtn:hover {"
        "  background-color: %9;"
        "}")
        .arg(c.surface.name(),                         // %1
             c.onSurface.name(),                      // %2
             c.secondaryContainer.name(),             // %3
             c.onSecondaryContainer.name(),           // %4
             c.surfaceContainerLow.name(),            // %5
             ThemeManager::rgba(c.outlineVariant, 0.4f), // %6
             c.primary.name(),                        // %7
             c.onPrimary.name(),                      // %8
             ThemeManager::rgba(c.primary, 0.85f)));  // %9
}
