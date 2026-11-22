// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/WebProfileRegistry.h"

#include "app/WebProviderProfile.h"
#include "core/ProviderRegistry.h"

WebProfileRegistry::WebProfileRegistry(ProviderRegistry &registry, OverrideFiles &files, QObject *parent)
    : QObject(parent)
    , m_registry(registry)
    , m_files(files)
{
    connect(&m_registry, &ProviderRegistry::providerUpdated, this, &WebProfileRegistry::applyUpdate);
}

WebProviderProfile *WebProfileRegistry::profileFor(const QString &providerId)
{
    if (WebProviderProfile *existing = m_profiles.value(providerId)) {
        return existing;
    }
    const std::optional<Provider> provider = m_registry.find(providerId);
    if (!provider || provider->kind != ProviderKind::Web) {
        return nullptr;
    }
    auto *profile = new WebProviderProfile(*provider, m_files, m_registry, this);
    m_profiles.insert(providerId, profile);
    return profile;
}

void WebProfileRegistry::applyUpdate(const QString &providerId)
{
    WebProviderProfile *profile = m_profiles.value(providerId);
    const std::optional<Provider> provider = m_registry.find(providerId);
    if (profile && provider) {
        profile->applyProvider(*provider);
    }
}
