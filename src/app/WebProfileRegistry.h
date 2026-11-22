// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QObject>

class OverrideFiles;
class ProviderRegistry;
class WebProviderProfile;

class WebProfileRegistry : public QObject
{
    Q_OBJECT

public:
    WebProfileRegistry(ProviderRegistry &registry, OverrideFiles &files, QObject *parent = nullptr);

    Q_INVOKABLE WebProviderProfile *profileFor(const QString &providerId);

private:
    void applyUpdate(const QString &providerId);

    ProviderRegistry &m_registry;
    OverrideFiles &m_files;
    QHash<QString, WebProviderProfile *> m_profiles;
};
