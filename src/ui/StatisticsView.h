#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QHash>

class StatisticsView : public QWidget
{
    Q_OBJECT
public:
    explicit StatisticsView(QWidget *parent = nullptr);
    void refreshStats();

signals:
    void backRequested();

private slots:
    void updateThemeColors();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupUi();
    QWidget* createStatCard(const QString& key, const QString& title, const QString& icon);
    QGridLayout* addSection(const QString& title, int columns);
    void setValue(const QString& key, int value);
    void setValue(const QString& key, const QString& value);
    static int countQuery(const QString& sql);
    static QString relativeDate(long long epochSecs);

    QVBoxLayout *m_statsLayout;
    QHash<QString, QLabel*> m_valueLabels;
};

#endif // STATISTICSVIEW_H
