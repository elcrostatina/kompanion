// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/Provider.h"

#include <QObject>
#include <QUrl>

#include <functional>

class OverrideFiles;
class ProfileStorage;
class ProviderRegistry;
class SecretStore;
class WebProfileRegistry;

class ProviderEditor : public QObject
{
    Q_OBJECT

public:
    ProviderEditor(ProviderRegistry &registry,
                   OverrideFiles &overrideFiles,
                   ProfileStorage &profileStorage,
                   WebProfileRegistry &webProfiles,
                   SecretStore &secrets,
                   QObject *parent = nullptr);

    Q_INVOKABLE void addProvider(const QString &kind, const QString &name, const QString &url, const QString &iconName);
    Q_INVOKABLE void remove(const QString &providerId);
    Q_INVOKABLE void moveUp(const QString &providerId);
    Q_INVOKABLE void moveDown(const QString &providerId);

    Q_INVOKABLE void setEnabled(const QString &providerId, bool enabled);
    Q_INVOKABLE void setName(const QString &providerId, const QString &name);
    Q_INVOKABLE void setUrl(const QString &providerId, const QString &url);
    Q_INVOKABLE void setIconName(const QString &providerId, const QString &iconName);
    Q_INVOKABLE void setUserAgent(const QString &providerId, const QString &userAgent);
    Q_INVOKABLE void setOverridesEnabled(const QString &providerId, bool enabled);

    Q_INVOKABLE void editCss(const QString &providerId);
    Q_INVOKABLE void editJavaScript(const QString &providerId);
    Q_INVOKABLE void resetCss(const QString &providerId);
    Q_INVOKABLE void clearSiteData(const QString &providerId);

    Q_INVOKABLE void checkApiKey(const QString &providerId);
    Q_INVOKABLE void storeApiKey(const QString &providerId, const QString &apiKey);
    Q_INVOKABLE void removeApiKey(const QString &providerId);

Q_SIGNALS:
    void apiKeyChecked(const QString &providerId, bool present);
    void siteDataCleared(const QString &providerId);
    void providerRemoved(const QString &providerId);

private:
    void modify(const QString &providerId, const std::function<void(Provider &)> &change);
    void moveBy(const QString &providerId, int offset);
    void openInTextEditor(const QString &path);
    static Provider newProvider(const QString &kind, const QString &name, const QString &url, const QString &iconName);

    ProviderRegistry &m_registry;
    OverrideFiles &m_overrideFiles;
    ProfileStorage &m_profileStorage;
    WebProfileRegistry &m_webProfiles;
    SecretStore &m_secrets;
};
