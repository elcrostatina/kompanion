// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ChatControllerRegistry.h"

#include "app/ChatController.h"
#include "core/ProviderRegistry.h"

ChatControllerRegistry::ChatControllerRegistry(ChatStore &store,
                                               ProviderRegistry &registry,
                                               AppSettings &settings,
                                               SecretStore &secrets,
                                               QNetworkAccessManager &network,
                                               QObject *parent)
    : QObject(parent)
    , m_store(store)
    , m_registry(registry)
    , m_settings(settings)
    , m_secrets(secrets)
    , m_network(network)
{
}

ChatController *ChatControllerRegistry::controllerFor(const QString &providerId)
{
    if (ChatController *existing = m_controllers.value(providerId)) {
        return existing;
    }
    const std::optional<Provider> provider = m_registry.find(providerId);
    if (!provider || provider->kind != ProviderKind::Local) {
        return nullptr;
    }
    auto *controller = new ChatController(providerId, m_store, m_registry, m_settings, m_secrets, m_network, this);
    m_controllers.insert(providerId, controller);
    return controller;
}
