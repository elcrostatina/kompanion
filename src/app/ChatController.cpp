// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ChatController.h"

#include "core/AppSettings.h"
#include "core/ChatExporter.h"
#include "core/ChatMessage.h"
#include "core/ChatStore.h"
#include "core/ProviderRegistry.h"
#include "core/SecretStore.h"
#include "core/TitleGenerator.h"

#include <KLocalizedString>

#include <QFile>
#include <QPointer>
#include <QSaveFile>

namespace {

constexpr QLatin1StringView MarkdownFormat{"markdown"};

}

ChatController::ChatController(const QString &providerId,
                               ChatStore &store,
                               ProviderRegistry &registry,
                               AppSettings &settings,
                               SecretStore &secrets,
                               QNetworkAccessManager &network,
                               QObject *parent)
    : QObject(parent)
    , m_providerId(providerId)
    , m_store(store)
    , m_registry(registry)
    , m_settings(settings)
    , m_secrets(secrets)
    , m_client(network)
    , m_chats(store, providerId)
    , m_systemPrompt(settings.defaultSystemPrompt())
{
    if (const std::optional<Provider> provider = registry.find(providerId)) {
        m_currentModel = provider->defaultModel;
    }
    connect(&m_client, &OpenAiClient::modelsReceived, this, &ChatController::applyModelList);
    connect(&m_client, &OpenAiClient::modelsFailed, this, [this](const ClientError &error) {
        m_loadingModels = false;
        Q_EMIT loadingModelsChanged();
        setError(error.message);
    });
    connect(&m_client, &OpenAiClient::chatDeltaReceived, this, &ChatController::handleDelta);
    connect(&m_client, &OpenAiClient::chatFinished, this, &ChatController::settleAssistantMessage);
    connect(&m_client, &OpenAiClient::chatFailed, this, &ChatController::handleFailure);
    connect(&m_registry, &ProviderRegistry::providerUpdated, this, [this](const QString &id) {
        if (id == m_providerId) {
            Q_EMIT providerChanged();
        }
    });
    reopenMostRecentChat();
}

QString ChatController::providerName() const
{
    const std::optional<Provider> provider = m_registry.find(m_providerId);
    return provider ? provider->name : m_providerId;
}

ChatListModel *ChatController::chats()
{
    return &m_chats;
}

MessageModel *ChatController::messages()
{
    return &m_messages;
}

qlonglong ChatController::currentChatId() const
{
    return m_currentChatId;
}

QString ChatController::currentTitle() const
{
    return m_currentTitle;
}

QString ChatController::systemPrompt() const
{
    return m_systemPrompt;
}

QStringList ChatController::models() const
{
    return m_models;
}

QString ChatController::currentModel() const
{
    return m_currentModel;
}

bool ChatController::isBusy() const
{
    return m_busy;
}

bool ChatController::isLoadingModels() const
{
    return m_loadingModels;
}

QString ChatController::errorText() const
{
    return m_errorText;
}

void ChatController::refreshModels()
{
    const std::optional<Provider> provider = m_registry.find(m_providerId);
    if (!provider || m_loadingModels) {
        return;
    }
    m_loadingModels = true;
    Q_EMIT loadingModelsChanged();
    QPointer<ChatController> self(this);
    m_secrets.read(SecretKeys::apiKeyFor(m_providerId), [self, url = provider->url](const std::optional<QString> &key) {
        if (self) {
            self->m_client.setEndpoint(url, key.value_or(QString()));
            self->m_client.listModels();
        }
    });
}

void ChatController::selectModel(const QString &model)
{
    setCurrentModel(model);
    if (m_currentChatId != 0) {
        m_store.setChatModel(m_currentChatId, model);
    }
    rememberDefaultModel(model);
}

void ChatController::newChat()
{
    if (!m_busy) {
        resetToDraft();
    }
}

void ChatController::openChat(qlonglong chatId)
{
    if (m_busy) {
        return;
    }
    if (const std::optional<ChatSummary> chat = m_store.chat(chatId)) {
        showChat(*chat);
    }
}

void ChatController::deleteChat(qlonglong chatId)
{
    if (m_busy && chatId == m_currentChatId) {
        return;
    }
    m_store.deleteChat(chatId);
    if (chatId == m_currentChatId) {
        resetToDraft();
    }
    reloadChatList();
}

void ChatController::renameChat(qlonglong chatId, const QString &title)
{
    m_store.renameChat(chatId, title.trimmed());
    if (chatId == m_currentChatId) {
        m_currentTitle = title.trimmed();
        Q_EMIT currentChatChanged();
    }
    reloadChatList();
}

void ChatController::search(const QString &text)
{
    m_searchText = text;
    reloadChatList();
}

void ChatController::sendMessage(const QString &text)
{
    const QString content = text.trimmed();
    if (content.isEmpty() || m_busy) {
        return;
    }
    if (m_currentModel.isEmpty()) {
        setError(i18n("No model is selected. Load a model in %1 and refresh the model list.", providerName()));
        return;
    }
    setError(QString());
    ensureChatExists();
    appendUserMessage(content);
    appendAssistantPlaceholder();
    setBusy(true);
    QPointer<ChatController> self(this);
    m_secrets.read(SecretKeys::apiKeyFor(m_providerId), [self](const std::optional<QString> &key) {
        if (self) {
            self->configureEndpointThenStream(key.value_or(QString()));
        }
    });
}

void ChatController::stop()
{
    m_client.cancelChat();
}

void ChatController::changeSystemPrompt(const QString &prompt)
{
    m_systemPrompt = prompt;
    if (m_currentChatId != 0) {
        m_store.setChatSystemPrompt(m_currentChatId, prompt);
    }
    Q_EMIT currentChatChanged();
}

void ChatController::exportChat(qlonglong chatId, const QString &format, const QUrl &target)
{
    const std::optional<ChatTranscript> transcript = m_store.transcript(chatId);
    if (!transcript) {
        return;
    }
    const QByteArray content = format == MarkdownFormat ? ChatExporter::toMarkdown(*transcript).toUtf8() : ChatExporter::toJson(*transcript);
    writeFile(target, content);
}

void ChatController::importChat(const QUrl &source)
{
    QFile file(source.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setError(i18n("Could not read %1.", source.toLocalFile()));
        return;
    }
    const std::optional<ChatTranscript> transcript = ChatExporter::fromJson(file.readAll());
    if (!transcript) {
        setError(i18n("%1 is not a valid chat file.", source.toLocalFile()));
        return;
    }
    const qint64 chatId = m_store.importTranscript(m_providerId, *transcript);
    reloadChatList();
    openChat(chatId);
}

void ChatController::dismissError()
{
    setError(QString());
}

void ChatController::configureEndpointThenStream(const QString &apiKey)
{
    const std::optional<Provider> provider = m_registry.find(m_providerId);
    if (!provider) {
        handleFailure({ClientError::Kind::ServerError, i18n("This provider no longer exists."), 0});
        return;
    }
    m_client.setEndpoint(provider->url, apiKey);
    m_client.streamChat(m_currentModel, buildHistory());
}

void ChatController::ensureChatExists()
{
    if (m_currentChatId != 0) {
        return;
    }
    m_currentChatId = m_store.createChat(m_providerId, m_currentModel, m_systemPrompt);
    Q_EMIT currentChatChanged();
    reloadChatList();
}

void ChatController::appendUserMessage(const QString &content)
{
    const qint64 messageId = m_store.appendMessage(m_currentChatId, QString(ChatRole::User), content);
    m_messages.appendMessage(messageId, QString(ChatRole::User), content);
    if (m_currentTitle.isEmpty()) {
        m_currentTitle = TitleGenerator::fromFirstMessage(content);
        m_store.renameChat(m_currentChatId, m_currentTitle);
        Q_EMIT currentChatChanged();
    }
    reloadChatList();
}

void ChatController::appendAssistantPlaceholder()
{
    m_assistantText.clear();
    m_assistantMessageId = m_store.appendMessage(m_currentChatId, QString(ChatRole::Assistant), QString());
    m_messages.appendMessage(m_assistantMessageId, QString(ChatRole::Assistant), QString());
}

QList<ChatMessage> ChatController::buildHistory() const
{
    QList<ChatMessage> history;
    if (!m_systemPrompt.trimmed().isEmpty()) {
        history.append({QString(ChatRole::System), m_systemPrompt});
    }
    const QList<StoredMessage> stored = m_store.messages(m_currentChatId);
    for (const StoredMessage &message : stored) {
        const bool isPlaceholder = message.id == m_assistantMessageId;
        if (!isPlaceholder && !message.content.isEmpty()) {
            history.append({message.role, message.content});
        }
    }
    return history;
}

void ChatController::applyModelList(const QStringList &modelIds)
{
    m_loadingModels = false;
    Q_EMIT loadingModelsChanged();
    m_models = modelIds;
    Q_EMIT modelsChanged();
    if (modelIds.isEmpty()) {
        setError(i18n("No model is loaded in %1. Load a model there, then refresh the list.", providerName()));
        return;
    }
    setError(QString());
    setCurrentModel(preferredModel());
}

QString ChatController::preferredModel() const
{
    return m_models.contains(m_currentModel) ? m_currentModel : m_models.constFirst();
}

void ChatController::reopenMostRecentChat()
{
    const QList<ChatSummary> existing = m_store.chats(m_providerId);
    if (!existing.isEmpty()) {
        showChat(existing.constFirst());
    }
}

void ChatController::showChat(const ChatSummary &chat)
{
    m_currentChatId = chat.id;
    m_currentTitle = chat.title;
    m_systemPrompt = chat.systemPrompt;
    QList<StoredMessage> visible = m_store.messages(chat.id);
    visible.removeIf([](const StoredMessage &message) {
        return message.role == ChatRole::Assistant && message.content.isEmpty();
    });
    m_messages.setMessages(visible);
    const bool modelStillAvailable = m_models.isEmpty() || m_models.contains(chat.model);
    if (!chat.model.isEmpty() && modelStillAvailable) {
        setCurrentModel(chat.model);
    }
    setError(QString());
    Q_EMIT currentChatChanged();
}

void ChatController::resetToDraft()
{
    m_currentChatId = 0;
    m_currentTitle.clear();
    m_systemPrompt = m_settings.defaultSystemPrompt();
    m_messages.setMessages({});
    setError(QString());
    Q_EMIT currentChatChanged();
}

void ChatController::settleAssistantMessage()
{
    if (m_assistantText.isEmpty()) {
        m_store.deleteMessage(m_assistantMessageId);
        m_messages.removeLastMessage();
    } else {
        m_store.updateMessageContent(m_assistantMessageId, m_assistantText);
    }
    m_assistantMessageId = 0;
    m_assistantText.clear();
    setBusy(false);
    reloadChatList();
}

void ChatController::handleDelta(const QString &text)
{
    m_assistantText += text;
    m_messages.setLastContent(m_assistantText);
}

void ChatController::handleFailure(const ClientError &error)
{
    setError(error.message);
    settleAssistantMessage();
}

void ChatController::setError(const QString &message)
{
    if (m_errorText != message) {
        m_errorText = message;
        Q_EMIT errorChanged();
    }
}

void ChatController::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        Q_EMIT busyChanged();
    }
}

void ChatController::setCurrentModel(const QString &model)
{
    if (m_currentModel != model) {
        m_currentModel = model;
        Q_EMIT currentModelChanged();
    }
}

void ChatController::rememberDefaultModel(const QString &model)
{
    std::optional<Provider> provider = m_registry.find(m_providerId);
    if (provider) {
        provider->defaultModel = model;
        m_registry.update(*provider);
    }
}

void ChatController::reloadChatList()
{
    m_chats.reload(m_searchText);
}

void ChatController::writeFile(const QUrl &target, const QByteArray &content)
{
    QSaveFile file(target.toLocalFile());
    const bool written = file.open(QIODevice::WriteOnly) && file.write(content) == content.size() && file.commit();
    if (!written) {
        setError(i18n("Could not write %1.", target.toLocalFile()));
    }
}
