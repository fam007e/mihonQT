#include "SidebarWidget.h"

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SidebarWidget::setupUi()
{
    setFixedWidth(80); // Navigation rail exact width

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 20, 0, 20);
    m_layout->setSpacing(10); // Space between items
    m_layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    m_libraryBtn = createNavButton("Library", "library", 0);
    m_updatesBtn = createNavButton("Updates", "updates", 1);
    m_historyBtn = createNavButton("History", "history", 2);
    m_browseBtn = createNavButton("Browse", "browse", 3);

    m_layout->addWidget(m_libraryBtn);
    m_layout->addWidget(m_updatesBtn);
    m_layout->addWidget(m_historyBtn);
    m_layout->addWidget(m_browseBtn);

    m_layout->addStretch();

    m_moreBtn = createNavButton("More", "more", 4);
    m_layout->addWidget(m_moreBtn);
}

QToolButton* SidebarWidget::createNavButton(const QString &text, const QString &iconName, int index)
{
    QToolButton *btn = new QToolButton(this);
    btn->setText(text);
    btn->setIcon(QIcon(QString(":/icons/%1.svg").arg(iconName)));
    btn->setIconSize(QSize(24, 24));
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setCheckable(true);
    btn->setAutoExclusive(true);
    btn->setFixedSize(70, 60); // Size for the rail item
    btn->setCursor(Qt::PointingHandCursor);

    // Navigation Rail pill style
    btn->setStyleSheet(
        "QToolButton { "
        "   border: none; "
        "   color: #D8DEE9; " // Inactive text
        "   font-size: 11px; "
        "   font-weight: 500; "
        "   border-radius: 16px; " // Pill shape for icon background
        "   padding: 4px; "
        "}"
        "QToolButton:checked { "
        "   background-color: #88C0D0; " // Pill background (primary color)
        "   color: #2E3440; "            // Active text color
        "}"
        "QToolButton:hover { "
        "   background-color: rgba(136, 192, 208, 0.15); "
        "}"
    );

    if (index == 0) btn->setChecked(true); // Default

    connect(btn, &QToolButton::clicked, this, [this, index]() {
        emit navigationRequested(index);
    });

    return btn;
}
