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
#include "../tracking/TrackingService.h"

class TrackingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrackingWidget(long mangaId, QWidget *parent = nullptr);
    void setMangaId(long mangaId);

signals:
    void closeRequested();

private slots:
    void onLoginClicked();
    void onSearchClicked();
    void onBindClicked();
    void onUpdateClicked();
    void onSearchResults(const QList<TrackingSearchResult>& results);

private:
    void setupUi();
    void refreshState();

    long m_mangaId;
    TrackingService *m_tracker;

    QLabel *m_statusLabel;
    QPushButton *m_loginBtn;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchBtn;
    QListWidget *m_resultsList;
    QComboBox *m_statusCombo;
    QSpinBox *m_chaptersSpin;
    QPushButton *m_updateBtn;

    QList<TrackingSearchResult> m_searchResults;
};

#endif // TRACKINGWIDGET_H
