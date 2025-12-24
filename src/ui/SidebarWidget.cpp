#include "SidebarWidget.h"

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SidebarWidget::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 10, 0, 10);
    m_layout->setSpacing(5);
    m_layout->setAlignment(Qt::AlignTop);

    // Style for the sidebar
    // setStyleSheet("background-color: #2E3440;"); // Nord Darker

    m_libraryBtn = createNavButton("Library", "library", 0);
    m_updatesBtn = createNavButton("Updates", "updates", 1);
    m_historyBtn = createNavButton("History", "history", 2);
    m_browseBtn = createNavButton("Browse", "browse", 3);

    m_layout->addWidget(m_libraryBtn);
    m_layout->addWidget(m_updatesBtn);
    m_layout->addWidget(m_historyBtn);
    m_layout->addWidget(m_browseBtn);

    m_layout->addStretch(); // Push "Settings" to bottom

    m_settingsBtn = createNavButton("Settings", "settings", 4);
    m_layout->addWidget(m_settingsBtn);
}

QPushButton* SidebarWidget::createNavButton(const QString &text, const QString &iconName, int index)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setCheckable(true);
    btn->setAutoExclusive(true);
    btn->setFixedHeight(50);
    btn->setCursor(Qt::PointingHandCursor);

    // Placeholder stylesheet - can be moved to ThemeManager later
    btn->setStyleSheet(
        "QPushButton { "
        "   text-align: left; "
        "   padding-left: 20px; "
        "   border: none; "
        "   color: #D8DEE9; "
        "   font-size: 14px; "
        "}"
        "QPushButton:checked { "
        "   background-color: #4C566A; "
        "   color: #ECEFF4; "
        "   border-left: 4px solid #88C0D0; "
        "}"
        "QPushButton:hover { "
        "   background-color: #434C5E; "
        "}"
    );

    if (index == 0) btn->setChecked(true); // Default

    connect(btn, &QPushButton::clicked, this, [this, index]() {
        emit navigationRequested(index);
    });

    return btn;
}
