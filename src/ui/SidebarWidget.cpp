#include "SidebarWidget.h"
#include "ThemeManager.h"

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme) {
        restyle();
    });
    restyle();
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

    if (index == 0) btn->setChecked(true); // Default

    connect(btn, &QToolButton::clicked, this, [this, index]() {
        emit navigationRequested(index);
    });

    return btn;
}

void SidebarWidget::restyle()
{
    // M3 Navigation Rail (Mihon): rounded pill behind the icon of the
    // selected item, colored with secondaryContainer like the nav bar.
    const M3Colors& c = ThemeManager::instance().colors();
    QString qss = QStringLiteral(
        "QToolButton { "
        "   border: none; "
        "   color: %1; "                  // onSurfaceVariant for inactive
        "   font-size: 11px; "
        "   font-weight: 500; "
        "   border-radius: 16px; "
        "   padding: 4px; "
        "   background: transparent; "
        "}"
        "QToolButton:checked { "
        "   background-color: %4; "       // secondaryContainer pill (Mihon nav bar)
        "   color: %2; "                  // onSecondaryContainer
        "}"
        "QToolButton:hover:!checked { "
        "   background-color: %3; "
        "}")
        .arg(c.onSurfaceVariant.name(),
             c.onSecondaryContainer.name(),
             ThemeManager::rgba(c.onSurface, 0.12f),
             c.secondaryContainer.name());
    for (QToolButton *b : findChildren<QToolButton*>())
        b->setStyleSheet(qss);
}
