#include "MoreView.h"
#include "../config/PreferenceManager.h"
#include "ThemeManager.h"
#include <QCheckBox>
#include <QDesktopServices>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QUrl>

MoreView::MoreView(QWidget *parent) : QWidget(parent) {
  setupUi();
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          &MoreView::updateThemeColors);
  updateThemeColors(); // Initial apply
}

void MoreView::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Scroll Area to mimic mobile feel
  m_scrollArea = new QScrollArea(this);
  QScrollArea *scrollArea = m_scrollArea;
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet("background-color: transparent;");

  QWidget *scrollContent = new QWidget();
  scrollContent->setStyleSheet("background-color: transparent;");
  m_listLayout = new QVBoxLayout(scrollContent);
  m_listLayout->setContentsMargins(0, 20, 0, 20);
  m_listLayout->setSpacing(4);
  m_listLayout->setAlignment(Qt::AlignTop);

  // Header with Logo (Tachiyomi/Mihon style)
  QWidget *headerContainer = new QWidget(this);
  headerContainer->setFixedHeight(160);
  QVBoxLayout *headerLayout = new QVBoxLayout(headerContainer);
  headerLayout->setAlignment(Qt::AlignCenter);

  // Container for the circular logo
  QWidget *logoCircle = new QWidget(headerContainer);
  logoCircle->setFixedSize(120, 120);
  logoCircle->setObjectName("logoCircle");

  // Layout for み and QT inside the circle
  QVBoxLayout *circleLayout = new QVBoxLayout(logoCircle);
  circleLayout->setContentsMargins(0, 0, 0, 0);
  circleLayout->setSpacing(0);
  circleLayout->setAlignment(Qt::AlignCenter);

  m_logoLabel = new QLabel("み", logoCircle);
  m_logoLabel->setAlignment(Qt::AlignCenter);

  m_qtBadge = new QLabel("QT", logoCircle);
  m_qtBadge->setAlignment(Qt::AlignCenter);

  circleLayout->addWidget(m_logoLabel);
  circleLayout->addWidget(m_qtBadge);

  headerLayout->addWidget(logoCircle);
  m_listLayout->addWidget(headerContainer);

  // Toggles Section
  m_downloadedOnlyToggle = addToggleItem(
      ":/icons/downloads.svg", "Downloaded only",
      "Only show manga that are downloaded",
      PreferenceManager::instance().value("downloadedOnly", false).toBool(),
      [this](bool checked) { emit downloadedOnlyChanged(checked); });

  m_incognitoToggle = addToggleItem(
      ":/icons/history.svg", "Incognito mode", "Pause history tracking",
      PreferenceManager::instance().value("security/incognito", false).toBool(),
      [this](bool checked) { emit incognitoChanged(checked); });

  addSeparator();

  // Actions Section
  addActionItem(":/icons/downloads.svg", "Download queue", "",
                [this]() { emit downloadsRequested(); });
  addActionItem(":/icons/categories.svg", "Categories", "",
                [this]() { emit categoriesRequested(); });
  addActionItem(":/icons/statistics.svg", "Statistics", "",
                [this]() { emit statisticsRequested(); });
  addActionItem(":/icons/downloads.svg", "Data and storage", "",
                [this]() { emit dataStorageRequested(); });
  addActionItem(":/icons/extensions.svg", "Extensions", "",
                [this]() { emit extensionsRequested(); });

  addSeparator();

  addActionItem(":/icons/settings.svg", "Settings", "",
                [this]() { emit settingsRequested(); });
  addActionItem(":/icons/about.svg", "About", "",
                [this]() { emit aboutRequested(); });
  addActionItem(":/icons/help.svg", "Help", "",
                [this]() { emit helpRequested(); });
  addActionItem(":/icons/public.svg", "Donate", "", [this]() {
    QDesktopServices::openUrl(QUrl("https://mihon.app/donate/"));
  });

  scrollArea->setWidget(scrollContent);
  mainLayout->addWidget(scrollArea);
}

void MoreView::addActionItem(const QString &iconPath, const QString &title,
                             const QString &subtitle,
                             const std::function<void()> &callback) {
  QWidget *scrollContent = qobject_cast<QWidget *>(m_listLayout->parent());
  QPushButton *btn = new QPushButton(scrollContent);
  m_rowWidgets.append(btn);
  btn->installEventFilter(this);
  const M3Colors &rc = ThemeManager::instance().colors();
  btn->setStyleSheet(
      QStringLiteral(
      "QPushButton { background-color: transparent; border: none; padding: "
      "12px 20px; text-align: left; }"
      "QPushButton:hover, QPushButton:focus { background-color: %1; }"
      "QPushButton:pressed { background-color: %2; }")
      .arg(ThemeManager::rgba(rc.onSurface, 0.08f),
           ThemeManager::rgba(rc.onSurface, 0.14f)));

  QHBoxLayout *hLayout = new QHBoxLayout(btn);
  hLayout->setContentsMargins(0, 0, 0, 0);
  hLayout->setSpacing(20);

  QLabel *iconLabel = new QLabel(btn);
  iconLabel->setPixmap(QIcon(iconPath).pixmap(24, 24));
  iconLabel->setFixedSize(24, 24);
  iconLabel->setStyleSheet("background: transparent;");

  QVBoxLayout *textLayout = new QVBoxLayout();
  textLayout->setSpacing(2);
  QLabel *titleLabel = new QLabel(title, btn);
  titleLabel->setStyleSheet("font-size: 16px; font-weight: 500; background: transparent;");
  textLayout->addWidget(titleLabel);

  if (!subtitle.isEmpty()) {
    QLabel *subLabel = new QLabel(subtitle, btn);
    subLabel->setStyleSheet(
        "font-size: 12px; color: palette(placeholder-text); background: transparent;");
    textLayout->addWidget(subLabel);
  }

  hLayout->addWidget(iconLabel);
  hLayout->addLayout(textLayout);
  hLayout->addStretch();

  connect(btn, &QPushButton::clicked, callback);
  m_listLayout->addWidget(btn);
}

QCheckBox *MoreView::addToggleItem(const QString &iconPath,
                                   const QString &title,
                                   const QString &subtitle, bool initialValue,
                                   const std::function<void(bool)> &callback) {
  QWidget *scrollContent = qobject_cast<QWidget *>(m_listLayout->parent());
  QWidget *container = new QWidget(scrollContent);
  container->setFocusPolicy(Qt::StrongFocus); // keyboard: Up/Down + Enter/Space
  m_rowWidgets.append(container);
  container->installEventFilter(this);
  container->setStyleSheet("background-color: transparent;");

  QHBoxLayout *hLayout = new QHBoxLayout(container);
  hLayout->setContentsMargins(20, 12, 20, 12);
  hLayout->setSpacing(20);

  QLabel *iconLabel = new QLabel(container);
  iconLabel->setPixmap(QIcon(iconPath).pixmap(24, 24));
  iconLabel->setFixedSize(24, 24);
  iconLabel->setStyleSheet("background: transparent;");

  QVBoxLayout *textLayout = new QVBoxLayout();
  textLayout->setSpacing(2);
  QLabel *titleLabel = new QLabel(title, container);
  // No explicit color: follows the palette (theme-safe)
  titleLabel->setStyleSheet("font-size: 16px; font-weight: 500; background: transparent;");
  textLayout->addWidget(titleLabel);

  if (!subtitle.isEmpty()) {
    QLabel *subLabel = new QLabel(subtitle, container);
    subLabel->setStyleSheet(
        "font-size: 12px; color: palette(placeholder-text); background: transparent;");
    textLayout->addWidget(subLabel);
  }

  QCheckBox *toggle = new QCheckBox(container);
  toggle->setChecked(initialValue);
  toggle->setCursor(Qt::PointingHandCursor);
  toggle->setStyleSheet(
      "QCheckBox { background: transparent; }"
      "QCheckBox::indicator { width: 48px; height: 28px; }"
      "QCheckBox::indicator:unchecked { image: url(:/icons/toggle_off.svg); }"
      "QCheckBox::indicator:checked { image: url(:/icons/toggle_on.svg); }");

  hLayout->addWidget(iconLabel);
  hLayout->addLayout(textLayout);
  hLayout->addStretch();
  hLayout->addWidget(toggle);

  connect(toggle, &QCheckBox::toggled, callback);
  m_listLayout->addWidget(container);
  return toggle;
}

// ------------------------------------------------------------------
// Keyboard navigation: Up/Down cycles rows, Enter/Space activates.
// Row containers (toggles) get a highlight when focused via eventFilter.
// ------------------------------------------------------------------
QList<QWidget *> MoreView::focusableRows() const
{
    QList<QWidget *> rows;
    for (QWidget *w : m_rowWidgets)
        if (w && w->isVisibleTo(const_cast<MoreView *>(this)))
            rows.append(w);
    return rows;
}

void MoreView::keyPressEvent(QKeyEvent *event)
{
    const QList<QWidget *> rows = focusableRows();
    QWidget *focus = focusWidget();
    int current = -1;
    for (int i = 0; i < rows.size(); ++i)
        if (rows[i] == focus) { current = i; break; }

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Left:
        if (!rows.isEmpty()) {
            current = (current <= 0) ? rows.size() - 1 : current - 1;
            rows[current]->setFocus();
            event->accept();
            return;
        }
        break;
    case Qt::Key_Down:
    case Qt::Key_Right:
        if (!rows.isEmpty()) {
            current = (current < 0 || current >= rows.size() - 1) ? 0 : current + 1;
            rows[current]->setFocus();
            event->accept();
            return;
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space: {
        // Toggle rows: flip their checkbox; buttons click themselves
        if (current >= 0) {
            QWidget *row = rows[current];
            if (QCheckBox *cb = row->findChild<QCheckBox *>()) {
                cb->toggle();
                event->accept();
                return;
            }
        }
        break;
    }
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}

bool MoreView::eventFilter(QObject *watched, QEvent *event)
{
    // Full-row highlight for keyboard focus (toggle rows are plain QWidgets)
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
        if (QWidget *w = qobject_cast<QWidget *>(watched)) {
            const M3Colors &c = ThemeManager::instance().colors();
            const bool focused = event->type() == QEvent::FocusIn;
            w->setStyleSheet(QStringLiteral("background-color: %1;")
                .arg(focused ? ThemeManager::rgba(c.onSurface, 0.10f)
                             : QStringLiteral("transparent")));
        }
        return false;
    }

    // Intercept keys ON the focused row: rows live inside a QScrollArea whose
    // key handler would otherwise eat Up/Down for scrolling before the events
    // ever reach MoreView::keyPressEvent.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Up:
        case Qt::Key_Left:
            moveRowFocus(-1);
            keyEvent->accept();
            return true;
        case Qt::Key_Down:
        case Qt::Key_Right:
            moveRowFocus(+1);
            keyEvent->accept();
            return true;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            if (QWidget *row = qobject_cast<QWidget *>(watched)) {
                if (auto *cb = row->findChild<QCheckBox *>()) {
                    cb->toggle(); // toggle row
                } else if (auto *btn = qobject_cast<QPushButton *>(row)) {
                    btn->click(); // action row
                }
            }
            keyEvent->accept();
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MoreView::moveRowFocus(int delta)
{
    const QList<QWidget *> rows = focusableRows();
    if (rows.isEmpty()) return;
    QWidget *focus = focusWidget();
    int current = -1;
    for (int i = 0; i < rows.size(); ++i)
        if (rows[i] == focus) { current = i; break; }

    // Wrap with clamping — never produce an out-of-range index
    // (Up on the first row used to compute -1 and segfault)
    if (current < 0)
        current = (delta < 0) ? rows.size() - 1 : 0;
    else
        current += delta;
    if (current < 0)
        current = rows.size() - 1;
    if (current >= rows.size())
        current = 0;

    rows[current]->setFocus();
    m_scrollArea->ensureWidgetVisible(rows[current]);
}

void MoreView::addSeparator() {
  QWidget *scrollContent = qobject_cast<QWidget *>(m_listLayout->parent());
  QFrame *line = new QFrame(scrollContent);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Plain);
  line->setStyleSheet("background-color: rgba(255, 255, 255, 0.1); max-height: "
                      "1px; margin: 8px 0;");
  m_listLayout->addWidget(line);
}

void MoreView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::RightButton) {
    emit backRequested();
  } else {
    QWidget::mousePressEvent(event);
  }
}

void MoreView::updatePreferences() {
  bool incognito =
      PreferenceManager::instance().value("security/incognito", false).toBool();
  if (m_incognitoToggle && m_incognitoToggle->isChecked() != incognito) {
    m_incognitoToggle->blockSignals(true);
    m_incognitoToggle->setChecked(incognito);
    m_incognitoToggle->blockSignals(false);
  }

  bool downloadedOnly =
      PreferenceManager::instance().value("downloadedOnly", false).toBool();
  if (m_downloadedOnlyToggle &&
      m_downloadedOnlyToggle->isChecked() != downloadedOnly) {
    m_downloadedOnlyToggle->blockSignals(true);
    m_downloadedOnlyToggle->setChecked(downloadedOnly);
    m_downloadedOnlyToggle->blockSignals(false);
  }
}

void MoreView::updateThemeColors() {
  // M3 tokens: circle = secondaryContainer, knock-out text = onSecondaryContainer
  // (the old palette.Window text failed contrast on every theme).
  const M3Colors &c = ThemeManager::instance().colors();
  QString accentColor = c.secondaryContainer.name();
  QString textColor = c.onSecondaryContainer.name();

  QWidget *logoCircle = findChild<QWidget *>("logoCircle");
  if (logoCircle) {
    logoCircle->setStyleSheet(QString("QWidget#logoCircle { "
                                      "  background-color: %1; "
                                      "  border-radius: 60px; "
                                      "}")
                                  .arg(accentColor));
  }

  if (m_logoLabel) {
    m_logoLabel->setStyleSheet(
        QString("font-size: 61px; font-weight: bold; color: %1; background: "
                "transparent; margin-bottom: 0px;")
            .arg(textColor));
  }

  if (m_qtBadge) {
    m_qtBadge->setStyleSheet(
        QString("font-size: 14px; font-weight: 800; color: %1; background: "
                "transparent; margin-top: -3px;")
            .arg(textColor));
  }
}
