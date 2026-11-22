// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ChatRecords.h"

#include <QSqlDatabase>

#include <functional>
#include <optional>

class ChatStore
{
public:
    using Clock = std::function<qint64()>;

    explicit ChatStore(const QString &databasePath, Clock clock = currentTimeMs);
    ~ChatStore();

    ChatStore(const ChatStore &) = delete;
    ChatStore &operator=(const ChatStore &) = delete;

    bool isOpen() const;

    qint64 createChat(const QString &providerId, const QString &model, const QString &systemPrompt);
    std::optional<ChatSummary> chat(qint64 chatId) const;
    QList<ChatSummary> chats(const QString &providerId, const QString &searchText = QString()) const;
    void renameChat(qint64 chatId, const QString &title);
    void setChatModel(qint64 chatId, const QString &model);
    void setChatSystemPrompt(qint64 chatId, const QString &systemPrompt);
    void deleteChat(qint64 chatId);

    qint64 appendMessage(qint64 chatId, const QString &role, const QString &content);
    void updateMessageContent(qint64 messageId, const QString &content);
    void deleteMessage(qint64 messageId);
    QList<StoredMessage> messages(qint64 chatId) const;

    std::optional<ChatTranscript> transcript(qint64 chatId) const;
    qint64 importTranscript(const QString &providerId, const ChatTranscript &transcript);

private:
    static qint64 currentTimeMs();
    static QString escapeLikePattern(const QString &text);

    bool openDatabase(const QString &databasePath);
    bool createSchema();
    bool execute(const QString &sql, const QVariantList &bindings = {}) const;
    void touchChat(qint64 chatId);

    QSqlDatabase m_database;
    Clock m_clock;
};
