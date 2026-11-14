#include "WebViewDialog.h"
#include "../network/NetworkAccessManager.h"
#include <QWebEngineCertificateError>
#include <QRegularExpression>
#include <QTimer>

WebViewDialog::WebViewDialog(const QString &url, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("WebView - " + url);
    resize(1024, 768);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Keep the browser's NATIVE identity — spoofing a Chrome version the
    // embedded Chromium isn't makes Cloudflare see contradictory UA/client
    // hints and loop "Verifying you are human" forever. Instead, sync the
    // extension network layer to the browser's real UA (minus the
    // QtWebEngine token, which some CDNs flag), so the challenge is solved
    // with the same identity the requests later replay.
    QWebEngineProfile *profile = m_page.profile();
    // The challenge iframe must be able to persist its clearance state:
    // force cookies persistent so Cloudflare's cf_clearance (and Turnstile's
    // internal state) survive page reloads. Third-party storage partitioning
    // is disabled process-wide in main.cpp.
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);

    QString ua = profile->httpUserAgent();
    static const QRegularExpression qtToken("QtWebEngine/[\\d.]+\\s*;?\\s*");
    ua.remove(qtToken);
    NetworkAccessManager::setUserAgentOverride(ua);

    m_webView = new QWebEngineView(this);
    m_webView->setPage(&m_page);

    // Some networks TLS-intercept the Cloudflare challenge backends, which
    // would otherwise leave Turnstile unable to complete (endless 403).
    // Accept overridable certificate errors here — and only here: this
    // dialog exists purely so the user can solve a bot challenge.
    connect(&m_page, &QWebEnginePage::certificateError, this,
            [](const QWebEngineCertificateError &error) {
                // the signal delivers a const ref; acceptCertificate() is
                // non-const, so act on a copy (the documented pattern)
                QWebEngineCertificateError copy = error;
                if (copy.isOverridable()) {
                    copy.acceptCertificate();
                }
            });

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

    // Cloudflare sets cf_clearance once the challenge has been passed; that
    // is the signal to hand control back to the request that was blocked.
    // Close automatically instead of leaving the dialog open forever.
    if (!m_cleared && cookie.name() == "cf_clearance") {
        m_cleared = true;
        QTimer::singleShot(500, this, &QDialog::accept);
    }
}

QList<QNetworkCookie> WebViewDialog::getHarvestedCookies() const
{
    return m_harvestedCookies;
}
