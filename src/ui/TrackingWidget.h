#ifndef TRACKINGWIDGET_H
#define TRACKINGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "../tracking/TrackingService.h"

class TrackingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrackingWidget(long mangaId, const QString& mangaTitle = QString(), QWidget *parent = nullptr);
    void setManga(long mangaId, const QString& mangaTitle);
    void setMangaId(long mangaId);

signals:
    void closeRequested();

private slots:
    void onTrackerChanged(int index);
    void onLoginClicked();
    void onLogoutClicked();
    void onSearchClicked();
    void onBindClicked();
    void onUnbindClicked();
    void onUpdateClicked();
    void onSearchResults(const QList<TrackingSearchResult>& results);

private:
    void setupUi();
    void refreshState();
    void connectTrackerSignals();

    long m_mangaId;
    QString m_mangaTitle;
    TrackingService *m_tracker;

    QComboBox *m_trackerCombo;
    QLabel *m_statusLabel;
    QPushButton *m_loginBtn;
    QPushButton *m_logoutBtn;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchBtn;
    QListWidget *m_resultsList;
    QPushButton *m_bindBtn;
    QPushButton *m_unbindBtn;
    QComboBox *m_statusCombo;
    QSpinBox *m_chaptersSpin;
    QDoubleSpinBox *m_scoreSpin;
    QPushButton *m_updateBtn;

    QList<TrackingSearchResult> m_searchResults;
};

#endif // TRACKINGWIDGET_H
