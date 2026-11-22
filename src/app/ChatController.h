// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "app/ChatListModel.h"
#include "app/MessageModel.h"
#include "core/OpenAiClient.h"
#include "core/Provider.h"

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QUrl>

class AppSettings;
class ChatStore;
class ProviderRegistry;
class QNetworkAccessManager;
class SecretStore;

class ChatController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by ChatControllerRegistry")

    Q_PROPERTY(QString providerName READ providerName NOTIFY providerChanged FINAL)
    Q_PROPERTY(ChatListModel *chats READ chats CONSTANT FINAL)
    Q_PROPERTY(MessageModel *messages READ messages CONSTANT FINAL)
    Q_PROPERTY(qlonglong currentChatId READ currentChatId NOTIFY currentChatChanged FINAL)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentChatChanged FINAL)
    Q_PROPERTY(QString systemPrompt READ systemPrompt NOTIFY currentChatChanged FINAL)
    Q_PROPERTY(QStringList models READ models NOTIFY modelsChanged FINAL)
    Q_PROPERTY(QString currentModel READ currentModel NOTIFY currentModelChanged FINAL)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged FINAL)
    Q_PROPERTY(bool loadingModels READ isLoadingModels NOTIFY loadingModelsChanged FINAL)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorChanged FINAL)

public:
    ChatController(const QString &providerId,
                   ChatStore &store,
                   ProviderRegistry &registry,
                   AppSettings &settings,
                   SecretStore &secrets,
                   QNetworkAccessManager &network,
                   QObject *parent = nullptr);

    QString providerName() const;
    ChatListModel *chats();
    MessageModel *messages();
    qlonglong currentChatId() const;
    QString currentTitle() const;
    QString systemPrompt() const;
    QStringList models() const;
    QString currentModel() const;
    bool isBusy() const;
    bool isLoadingModels() const;
    QString errorText() const;

    Q_INVOKABLE void refreshModels();
    Q_INVOKABLE void selectModel(const QString &model);
    Q_INVOKABLE void newChat();
    Q_INVOKABLE void openChat(qlonglong chatId);
    Q_INVOKABLE void deleteChat(qlonglong chatId);
    Q_INVOKABLE void renameChat(qlonglong chatId, const QString &title);
    Q_INVOKABLE void search(const QString &text);
    Q_INVOKABLE void sendMessage(const QString &text);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void changeSystemPrompt(const QString &prompt);
    Q_INVOKABLE void exportChat(qlonglong chatId, const QString &format, const QUrl &target);
    Q_INVOKABLE void importChat(const QUrl &source);
    Q_INVOKABLE void dismissError();

Q_SIGNALS:
    void providerChanged();
    void currentChatChanged();
    void modelsChanged();
    void currentModelChanged();
    void busyChanged();
    void loadingModelsChanged();
    void errorChanged();

private:
    void configureEndpointThenStream(const QString &apiKey);
    void ensureChatExists();
    void appendUserMessage(const QString &content);
    void appendAssistantPlaceholder();
    QList<ChatMessage> buildHistory() const;
    void applyModelList(const QStringList &modelIds);
    QString preferredModel() const;
    void reopenMostRecentChat();
    void showChat(const ChatSummary &chat);
    void resetToDraft();
    void settleAssistantMessage();
    void handleDelta(const QString &text);
    void handleFailure(const ClientError &error);
    void setError(const QString &message);
    void setBusy(bool busy);
    void setCurrentModel(const QString &model);
    void rememberDefaultModel(const QString &model);
    void reloadChatList();
    void writeFile(const QUrl &target, const QByteArray &content);

    QString m_providerId;
    ChatStore &m_store;
    ProviderRegistry &m_registry;
    AppSettings &m_settings;
    SecretStore &m_secrets;
    OpenAiClient m_client;
    ChatListModel m_chats;
    MessageModel m_messages;
    qlonglong m_currentChatId = 0;
    QString m_currentTitle;
    QString m_systemPrompt;
    QStringList m_models;
    QString m_currentModel;
    QString m_searchText;
    QString m_errorText;
    bool m_busy = false;
    bool m_loadingModels = false;
    qint64 m_assistantMessageId = 0;
    QString m_assistantText;
};
