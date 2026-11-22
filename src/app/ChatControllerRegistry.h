// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QObject>

class AppSettings;
class ChatController;
class ChatStore;
class ProviderRegistry;
class QNetworkAccessManager;
class SecretStore;

class ChatControllerRegistry : public QObject
{
    Q_OBJECT

public:
    ChatControllerRegistry(ChatStore &store,
                           ProviderRegistry &registry,
                           AppSettings &settings,
                           SecretStore &secrets,
                           QNetworkAccessManager &network,
                           QObject *parent = nullptr);

    Q_INVOKABLE ChatController *controllerFor(const QString &providerId);

private:
    ChatStore &m_store;
    ProviderRegistry &m_registry;
    AppSettings &m_settings;
    SecretStore &m_secrets;
    QNetworkAccessManager &m_network;
    QHash<QString, ChatController *> m_controllers;
};
