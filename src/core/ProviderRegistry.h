// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/Provider.h"

#include <KSharedConfig>

#include <QList>
#include <QObject>

#include <optional>

class ProviderRegistry : public QObject
{
    Q_OBJECT

public:
    explicit ProviderRegistry(KSharedConfigPtr config, QObject *parent = nullptr);

    void seedMissingPresets();

    QList<Provider> providers() const;
    QList<Provider> enabledProviders() const;
    std::optional<Provider> find(const QString &id) const;

    QString add(Provider provider);
    void update(const Provider &provider);
    void remove(const QString &id);
    void move(int from, int to);

Q_SIGNALS:
    void providersChanged();
    void providerUpdated(const QString &id);

private:
    void load();
    QStringList storedIdsInOrder() const;
    Provider readProvider(const QString &id) const;
    void writeProvider(const Provider &provider);
    void writeOrder();
    void flush();
    qsizetype indexOf(const QString &id) const;
    qsizetype insertionIndexFor(ProviderKind kind) const;

    KSharedConfigPtr m_config;
    QList<Provider> m_providers;
};
