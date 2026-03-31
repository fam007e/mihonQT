#ifndef WEBVIEWDIALOG_H
#define WEBVIEWDIALOG_H

#include <QDialog>
#include <QWebEngineView>
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
    QWebEngineView *m_webView;
    QList<QNetworkCookie> m_harvestedCookies;

private slots:
    void onCookieAdded(const QNetworkCookie &cookie);
};

#endif // WEBVIEWDIALOG_H
