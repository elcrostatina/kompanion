// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ProviderEditor.h"

#include "app/WebProfileRegistry.h"
#include "app/WebProviderProfile.h"
#include "core/EmptyFile.h"
#include "core/OverrideFiles.h"
#include "core/ProfileStorage.h"
#include "core/ProviderRegistry.h"
#include "core/SecretStore.h"

#include <KIO/OpenUrlJob>

#include <QFile>
#include <QPointer>

namespace {

constexpr QLatin1StringView PlainTextMimeType{"text/plain"};

}

ProviderEditor::ProviderEditor(ProviderRegistry &registry,
                               OverrideFiles &overrideFiles,
                               ProfileStorage &profileStorage,
                               WebProfileRegistry &webProfiles,
                               SecretStore &secrets,
                               QObject *parent)
    : QObject(parent)
    , m_registry(registry)
    , m_overrideFiles(overrideFiles)
    , m_profileStorage(profileStorage)
    , m_webProfiles(webProfiles)
    , m_secrets(secrets)
{
}

void ProviderEditor::addProvider(const QString &kind, const QString &name, const QString &url, const QString &iconName)
{
    if (!name.trimmed().isEmpty() && !url.trimmed().isEmpty()) {
        m_registry.add(newProvider(kind, name.trimmed(), url.trimmed(), iconName.trimmed()));
    }
}

void ProviderEditor::remove(const QString &providerId)
{
    m_profileStorage.scheduleWipe(providerId);
    m_secrets.remove(SecretKeys::apiKeyFor(providerId), [](bool) {});
    m_registry.remove(providerId);
    Q_EMIT providerRemoved(providerId);
}

void ProviderEditor::moveUp(const QString &providerId)
{
    moveBy(providerId, -1);
}

void ProviderEditor::moveDown(const QString &providerId)
{
    moveBy(providerId, 1);
}

void ProviderEditor::setEnabled(const QString &providerId, bool enabled)
{
    modify(providerId, [enabled](Provider &provider) { provider.enabled = enabled; });
}

void ProviderEditor::setName(const QString &providerId, const QString &name)
{
    if (!name.trimmed().isEmpty()) {
        modify(providerId, [&name](Provider &provider) { provider.name = name.trimmed(); });
    }
}

void ProviderEditor::setUrl(const QString &providerId, const QString &url)
{
    if (!url.trimmed().isEmpty()) {
        modify(providerId, [&url](Provider &provider) { provider.url = QUrl::fromUserInput(url.trimmed()); });
    }
}

void ProviderEditor::setIconName(const QString &providerId, const QString &iconName)
{
    modify(providerId, [&iconName](Provider &provider) { provider.iconName = iconName.trimmed(); });
}

void ProviderEditor::setUserAgent(const QString &providerId, const QString &userAgent)
{
    modify(providerId, [&userAgent](Provider &provider) { provider.userAgent = userAgent.trimmed(); });
}

void ProviderEditor::setOverridesEnabled(const QString &providerId, bool enabled)
{
    modify(providerId, [enabled](Provider &provider) { provider.overridesEnabled = enabled; });
}

void ProviderEditor::editCss(const QString &providerId)
{
    m_overrideFiles.ensureCssExists(providerId);
    openInTextEditor(m_overrideFiles.cssPath(providerId));
}

void ProviderEditor::editJavaScript(const QString &providerId)
{
    const QString path = m_overrideFiles.javaScriptPath(providerId);
    if (!QFile::exists(path)) {
        EmptyFile::create(path);
    }
    openInTextEditor(path);
}

void ProviderEditor::resetCss(const QString &providerId)
{
    m_overrideFiles.resetCss(providerId);
}

void ProviderEditor::clearSiteData(const QString &providerId)
{
    if (WebProviderProfile *profile = m_webProfiles.profileFor(providerId)) {
        profile->clearBrowsingData();
    }
    m_profileStorage.scheduleWipe(providerId);
    Q_EMIT siteDataCleared(providerId);
}

void ProviderEditor::checkApiKey(const QString &providerId)
{
    QPointer<ProviderEditor> self(this);
    m_secrets.read(SecretKeys::apiKeyFor(providerId), [self, providerId](const std::optional<QString> &key) {
        if (self) {
            Q_EMIT self->apiKeyChecked(providerId, key.has_value() && !key->isEmpty());
        }
    });
}

void ProviderEditor::storeApiKey(const QString &providerId, const QString &apiKey)
{
    QPointer<ProviderEditor> self(this);
    m_secrets.write(SecretKeys::apiKeyFor(providerId), apiKey, [self, providerId](bool) {
        if (self) {
            self->checkApiKey(providerId);
        }
    });
}

void ProviderEditor::removeApiKey(const QString &providerId)
{
    QPointer<ProviderEditor> self(this);
    m_secrets.remove(SecretKeys::apiKeyFor(providerId), [self, providerId](bool) {
        if (self) {
            self->checkApiKey(providerId);
        }
    });
}

void ProviderEditor::modify(const QString &providerId, const std::function<void(Provider &)> &change)
{
    std::optional<Provider> provider = m_registry.find(providerId);
    if (provider) {
        change(*provider);
        m_registry.update(*provider);
    }
}

void ProviderEditor::moveBy(const QString &providerId, int offset)
{
    const QList<Provider> providers = m_registry.providers();
    for (int index = 0; index < providers.size(); ++index) {
        if (providers.at(index).id == providerId) {
            m_registry.move(index, index + offset);
            return;
        }
    }
}

void ProviderEditor::openInTextEditor(const QString &path)
{
    auto *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(path), QString(PlainTextMimeType), this);
    job->start();
}

Provider ProviderEditor::newProvider(const QString &kind, const QString &name, const QString &url, const QString &iconName)
{
    Provider provider;
    provider.name = name;
    provider.kind = providerKindFromName(kind);
    provider.url = QUrl::fromUserInput(url);
    provider.iconName = iconName;
    return provider;
}
