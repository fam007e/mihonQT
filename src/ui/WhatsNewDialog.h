#ifndef WHATSNEWDIALOG_H
#define WHATSNEWDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>

class WhatsNewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WhatsNewDialog(QWidget *parent = nullptr);

private:
    void setupUi();
    void restyle();

    QLabel *m_titleLabel;
    QLabel *m_versionBadge;
    QTextBrowser *m_textBrowser;
    QPushButton *m_closeButton;
};

#endif // WHATSNEWDIALOG_H
