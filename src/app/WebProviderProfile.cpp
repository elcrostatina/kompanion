// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/WebProviderProfile.h"

#include "core/AppPaths.h"
#include "core/BaseStyle.h"
#include "core/FileNaming.h"
#include "core/OverrideFiles.h"
#include "core/OverrideScripts.h"
#include "core/PageColorScript.h"
#include "core/ProviderRegistry.h"
#include "core/SiteDataScripts.h"
#include "core/UserAgent.h"

#include <QDir>
#include <QQuickWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QQuickWebEngineDownloadRequest>

namespace {

constexpr QLatin1StringView CacheFolder{"cache"};

}

WebProviderProfile::WebProviderProfile(const Provider &provider, OverrideFiles &files, ProviderRegistry &registry, QObject *parent)
    : QObject(parent)
    , m_provider(provider)
    , m_files(files)
    , m_registry(registry)
    , m_profile(new QQuickWebEngineProfile(provider.id, this))
    , m_navigationPolicy(provider.url)
    , m_reloader(files, provider.id, this)
    , m_defaultUserAgent(UserAgent::withoutEngineToken(m_profile->httpUserAgent()))
{
    m_files.ensureCssExists(m_provider.id);
    configureStorage();
    configureUserAgent();
    refreshScripts();
    connect(&m_reloader, &OverrideReloader::filesChanged, this, &WebProviderProfile::refreshScripts);
    connect(m_profile, &QQuickWebEngineProfile::downloadRequested, this, &WebProviderProfile::saveDownload);
}

void WebProviderProfile::applyProvider(const Provider &provider)
{
    if (provider == m_provider) {
        return;
    }
    m_provider = provider;
    m_navigationPolicy = NavigationPolicy(provider.url);
    configureUserAgent();
    refreshScripts();
    Q_EMIT providerChanged();
}

QQuickWebEngineProfile *WebProviderProfile::profile() const
{
    return m_profile;
}

QString WebProviderProfile::providerId() const
{
    return m_provider.id;
}

QString WebProviderProfile::providerName() const
{
    return m_provider.name;
}

QUrl WebProviderProfile::startUrl() const
{
    return m_provider.url;
}

double WebProviderProfile::zoomFactor() const
{
    return m_provider.zoomFactor;
}

QString WebProviderProfile::styleScript() const
{
    return m_styleScript;
}

QString WebProviderProfile::userScript() const
{
    return m_userScript;
}

QString WebProviderProfile::pageColorScript() const
{
    return PageColorScript::sampleBackground();
}

void WebProviderProfile::rememberZoomFactor(double zoomFactor)
{
    Provider updated = m_provider;
    updated.zoomFactor = zoomFactor;
    m_registry.update(updated);
}

void WebProviderProfile::clearBrowsingData()
{
    m_profile->cookieStore()->deleteAllCookies();
    m_profile->clearHttpCache();
    Q_EMIT pageStorageClearRequested(SiteDataScripts::clearPageStorage());
}

bool WebProviderProfile::staysInsidePanel(const QUrl &target) const
{
    return m_navigationPolicy.staysInsidePanel(target);
}

void WebProviderProfile::configureStorage()
{
    const QString directory = AppPaths::profileDirectory(m_provider.id);
    m_profile->setPersistentStoragePath(directory);
    m_profile->setCachePath(QDir(directory).filePath(QString(CacheFolder)));
    m_profile->setPersistentCookiesPolicy(QQuickWebEngineProfile::ForcePersistentCookies);
    m_profile->setPersistentPermissionsPolicy(QQuickWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
}

void WebProviderProfile::configureUserAgent()
{
    const bool hasCustomUserAgent = !m_provider.userAgent.trimmed().isEmpty();
    m_profile->setHttpUserAgent(hasCustomUserAgent ? m_provider.userAgent : m_defaultUserAgent);
}

void WebProviderProfile::refreshScripts()
{
    const QString css = m_provider.overridesEnabled ? m_files.readCss(m_provider.id) : QString();
    const QString javaScript = m_provider.overridesEnabled ? m_files.readJavaScript(m_provider.id) : QString();
    const QString styleScript = OverrideScripts::styleScript(BaseStyle::css() + QLatin1Char('\n') + css);
    const QString userScript = OverrideScripts::userScript(javaScript);
    if (styleScript == m_styleScript && userScript == m_userScript) {
        return;
    }
    m_styleScript = styleScript;
    m_userScript = userScript;
    Q_EMIT scriptsChanged();
}

void WebProviderProfile::saveDownload(QQuickWebEngineDownloadRequest *download)
{
    const QDir directory(AppPaths::downloadsDirectory());
    QDir().mkpath(directory.absolutePath());
    download->setDownloadDirectory(directory.absolutePath());
    download->setDownloadFileName(FileNaming::uniqueFileName(directory, download->downloadFileName()));
    connect(download, &QWebEngineDownloadRequest::stateChanged, this, [this, download](QWebEngineDownloadRequest::DownloadState state) {
        if (state == QWebEngineDownloadRequest::DownloadCompleted) {
            Q_EMIT downloadSaved(QDir(download->downloadDirectory()).filePath(download->downloadFileName()));
        }
    });
    download->accept();
}
