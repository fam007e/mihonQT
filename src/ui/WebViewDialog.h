#ifndef WEBVIEWDIALOG_H
#define WEBVIEWDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QList>

class WebViewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WebViewDialog(const QString &url, QWidget *parent = nullptr);
    ~WebViewDialog();

    // Optionally expose the harvested cookies if running synchronously
    QList<QNetworkCookie> getHarvestedCookies() const;

private:
    // Own page so the dialog can tune certificate handling (see ctor) —
    // it exists purely to solve bot challenges.
    QWebEnginePage m_page;
    QWebEngineView *m_webView;
    QList<QNetworkCookie> m_harvestedCookies;
    bool m_cleared = false; // challenge passed -> dialog auto-closes once

private slots:
    void onCookieAdded(const QNetworkCookie &cookie);
};

#endif // WEBVIEWDIALOG_H
