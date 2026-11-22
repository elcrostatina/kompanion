// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ProviderModel.h"

#include "core/ProviderRegistry.h"

ProviderModel::ProviderModel(ProviderRegistry &registry, Filter filter, QObject *parent)
    : QAbstractListModel(parent)
    , m_registry(registry)
    , m_filter(filter)
    , m_providers(visibleProviders())
{
    connect(&m_registry, &ProviderRegistry::providersChanged, this, &ProviderModel::rebuild);
    connect(&m_registry, &ProviderRegistry::providerUpdated, this, &ProviderModel::refreshValues);
}

int ProviderModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_providers.size());
}

QVariant ProviderModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }
    const Provider &provider = m_providers.at(index.row());
    switch (role) {
    case IdRole:
        return provider.id;
    case NameRole:
        return provider.name;
    case KindRole:
        return providerKindName(provider.kind);
    case IconNameRole:
        return provider.iconName;
    case EnabledRole:
        return provider.enabled;
    case UrlRole:
        return provider.url;
    case UserAgentRole:
        return provider.userAgent;
    case OverridesEnabledRole:
        return provider.overridesEnabled;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ProviderModel::roleNames() const
{
    return {
        {IdRole, "providerId"},
        {NameRole, "name"},
        {KindRole, "kind"},
        {IconNameRole, "iconName"},
        {EnabledRole, "providerEnabled"},
        {UrlRole, "url"},
        {UserAgentRole, "userAgent"},
        {OverridesEnabledRole, "overridesEnabled"},
    };
}

QList<Provider> ProviderModel::visibleProviders() const
{
    return m_filter == Filter::EnabledOnly ? m_registry.enabledProviders() : m_registry.providers();
}

QStringList ProviderModel::idsOf(const QList<Provider> &providers) const
{
    QStringList ids;
    for (const Provider &provider : providers) {
        ids.append(provider.id);
    }
    return ids;
}

void ProviderModel::rebuild()
{
    beginResetModel();
    m_providers = visibleProviders();
    endResetModel();
}

void ProviderModel::refreshValues()
{
    const QList<Provider> latest = visibleProviders();
    if (idsOf(latest) != idsOf(m_providers)) {
        rebuild();
        return;
    }
    m_providers = latest;
    if (!m_providers.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(static_cast<int>(m_providers.size()) - 1));
    }
}
