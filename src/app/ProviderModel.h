// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/Provider.h"

#include <QAbstractListModel>

class ProviderRegistry;

class ProviderModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class Filter {
        AllProviders,
        EnabledOnly,
    };

    enum Role {
        IdRole = Qt::UserRole + 1,
        NameRole,
        KindRole,
        IconNameRole,
        EnabledRole,
        UrlRole,
        UserAgentRole,
        OverridesEnabledRole,
    };

    ProviderModel(ProviderRegistry &registry, Filter filter, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Provider> visibleProviders() const;
    QStringList idsOf(const QList<Provider> &providers) const;
    void rebuild();
    void refreshValues();

    ProviderRegistry &m_registry;
    Filter m_filter;
    QList<Provider> m_providers;
};
