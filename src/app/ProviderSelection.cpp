// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ProviderSelection.h"

#include "core/AppSettings.h"
#include "core/ProviderRegistry.h"

ProviderSelection::ProviderSelection(ProviderRegistry &registry, AppSettings &settings, QObject *parent)
    : QObject(parent)
    , m_registry(registry)
    , m_settings(settings)
    , m_currentId(settings.lastProviderId())
{
    connect(&m_registry, &ProviderRegistry::providersChanged, this, &ProviderSelection::keepSelectionValid);
    connect(&m_registry, &ProviderRegistry::providerUpdated, this, &ProviderSelection::keepSelectionValid);
    keepSelectionValid();
}

QString ProviderSelection::currentId() const
{
    return m_currentId;
}

int ProviderSelection::currentIndex() const
{
    return static_cast<int>(enabledIds().indexOf(m_currentId));
}

void ProviderSelection::selectId(const QString &providerId)
{
    if (providerId == m_currentId || !enabledIds().contains(providerId)) {
        return;
    }
    m_currentId = providerId;
    m_settings.setLastProviderId(providerId);
    Q_EMIT currentChanged();
}

void ProviderSelection::selectIndex(int index)
{
    const QStringList ids = enabledIds();
    if (index >= 0 && index < ids.size()) {
        selectId(ids.at(index));
    }
}

QStringList ProviderSelection::enabledIds() const
{
    QStringList ids;
    const QList<Provider> enabled = m_registry.enabledProviders();
    for (const Provider &provider : enabled) {
        ids.append(provider.id);
    }
    return ids;
}

void ProviderSelection::keepSelectionValid()
{
    const QStringList ids = enabledIds();
    if (ids.contains(m_currentId)) {
        Q_EMIT currentChanged();
        return;
    }
    m_currentId = ids.isEmpty() ? QString() : ids.constFirst();
    Q_EMIT currentChanged();
}
