#include "WebViewDialog.h"
#include "../network/NetworkAccessManager.h"
#include <QRegularExpression>

WebViewDialog::WebViewDialog(const QString &url, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("WebView - " + url);
    resize(1024, 768);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_webView = new QWebEngineView(this);

    QWebEngineProfile *profile = m_webView->page()->profile();
    // Keep the browser's NATIVE identity — spoofing a Chrome version the
    // embedded Chromium isn't makes Cloudflare see contradictory UA/client
    // hints and loop "Verifying you are human" forever. Instead, sync the
    // extension network layer to the browser's real UA (minus the
    // QtWebEngine token, which some CDNs flag), so the challenge is solved
    // with the same identity the requests later replay.
    QString ua = profile->httpUserAgent();
    static const QRegularExpression qtToken("QtWebEngine/[\\d.]+\\s*;?\\s*");
    ua.remove(qtToken);
    NetworkAccessManager::setUserAgentOverride(ua);

    QWebEngineCookieStore *cookieStore = profile->cookieStore();
    connect(cookieStore, &QWebEngineCookieStore::cookieAdded, this, &WebViewDialog::onCookieAdded);

    m_webView->load(QUrl(url));
    layout->addWidget(m_webView);
}

WebViewDialog::~WebViewDialog()
{
}

void WebViewDialog::onCookieAdded(const QNetworkCookie &cookie)
{
    m_harvestedCookies.append(cookie);
    // You could also emit a signal here for the NetworkManager to update universally
}

QList<QNetworkCookie> WebViewDialog::getHarvestedCookies() const
{
    return m_harvestedCookies;
}
