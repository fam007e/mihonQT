#include "WebViewDialog.h"
#include "../network/NetworkAccessManager.h"

WebViewDialog::WebViewDialog(const QString &url, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("WebView - " + url);
    resize(1024, 768);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_webView = new QWebEngineView(this);

    QWebEngineProfile *profile = m_webView->page()->profile();
    // Cloudflare binds cf_clearance to the User-Agent: the challenge must be
    // solved with the exact UA the extension network layer later replays,
    // otherwise the harvested cookies are rejected.
    profile->setHttpUserAgent(NetworkAccessManager::browserUserAgent());

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
