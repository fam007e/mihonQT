#ifndef STATISTICSVIEW_H
#define STATISTICSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>

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

private:
    void setupUi();
    QWidget* createStatCard(const QString& title, const QString& value, const QString& icon);

    QVBoxLayout *m_statsLayout;
    QLabel *m_totalMangaLabel;
    QLabel *m_totalChaptersLabel;
    QLabel *m_readChaptersLabel;
    QLabel *m_totalCategoriesLabel;
};

#endif // STATISTICSVIEW_H
