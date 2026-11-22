// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ChatStore.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

constexpr QLatin1StringView DriverName{"QSQLITE"};
constexpr QLatin1StringView InMemoryDatabase{":memory:"};
constexpr QLatin1StringView LikeEscapeCharacter{"\\"};

ChatSummary summaryFrom(const QSqlQuery &query)
{
    ChatSummary summary;
    summary.id = query.value(0).toLongLong();
    summary.providerId = query.value(1).toString();
    summary.title = query.value(2).toString();
    summary.model = query.value(3).toString();
    summary.systemPrompt = query.value(4).toString();
    summary.createdAt = query.value(5).toLongLong();
    summary.updatedAt = query.value(6).toLongLong();
    return summary;
}

StoredMessage messageFrom(const QSqlQuery &query)
{
    StoredMessage message;
    message.id = query.value(0).toLongLong();
    message.chatId = query.value(1).toLongLong();
    message.role = query.value(2).toString();
    message.content = query.value(3).toString();
    message.createdAt = query.value(4).toLongLong();
    return message;
}

QString nonNull(const QString &text)
{
    return text.isNull() ? QString(QLatin1StringView("")) : text;
}

const QString SummaryColumns = QStringLiteral("c.id, c.provider_id, c.title, c.model, c.system_prompt, c.created_at, c.updated_at");

}

ChatStore::ChatStore(const QString &databasePath, Clock clock)
    : m_clock(std::move(clock))
{
    if (openDatabase(databasePath)) {
        createSchema();
    }
}

ChatStore::~ChatStore()
{
    const QString connectionName = m_database.connectionName();
    m_database.close();
    m_database = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

bool ChatStore::isOpen() const
{
    return m_database.isOpen();
}

qint64 ChatStore::createChat(const QString &providerId, const QString &model, const QString &systemPrompt)
{
    const qint64 now = m_clock();
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("INSERT INTO chats (provider_id, title, model, system_prompt, created_at, updated_at) VALUES (?, '', ?, ?, ?, ?)"));
    query.addBindValue(providerId);
    query.addBindValue(nonNull(model));
    query.addBindValue(nonNull(systemPrompt));
    query.addBindValue(now);
    query.addBindValue(now);
    return query.exec() ? query.lastInsertId().toLongLong() : 0;
}

std::optional<ChatSummary> ChatStore::chat(qint64 chatId) const
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT %1 FROM chats c WHERE c.id = ?").arg(SummaryColumns));
    query.addBindValue(chatId);
    if (query.exec() && query.next()) {
        return summaryFrom(query);
    }
    return std::nullopt;
}

QList<ChatSummary> ChatStore::chats(const QString &providerId, const QString &searchText) const
{
    QList<ChatSummary> result;
    QSqlQuery query(m_database);
    const QString trimmed = searchText.trimmed();
    if (trimmed.isEmpty()) {
        query.prepare(QStringLiteral("SELECT %1 FROM chats c WHERE c.provider_id = ? ORDER BY c.updated_at DESC, c.id DESC").arg(SummaryColumns));
        query.addBindValue(providerId);
    } else {
        query.prepare(QStringLiteral("SELECT %1 FROM chats c WHERE c.provider_id = ? AND (c.title LIKE ? ESCAPE '%2' OR EXISTS "
                                     "(SELECT 1 FROM messages m WHERE m.chat_id = c.id AND m.content LIKE ? ESCAPE '%2')) "
                                     "ORDER BY c.updated_at DESC, c.id DESC")
                          .arg(SummaryColumns, QString(LikeEscapeCharacter)));
        const QString pattern = QLatin1Char('%') + escapeLikePattern(trimmed) + QLatin1Char('%');
        query.addBindValue(providerId);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    }
    if (query.exec()) {
        while (query.next()) {
            result.append(summaryFrom(query));
        }
    }
    return result;
}

void ChatStore::renameChat(qint64 chatId, const QString &title)
{
    execute(QStringLiteral("UPDATE chats SET title = ? WHERE id = ?"), {nonNull(title), chatId});
}

void ChatStore::setChatModel(qint64 chatId, const QString &model)
{
    execute(QStringLiteral("UPDATE chats SET model = ? WHERE id = ?"), {nonNull(model), chatId});
}

void ChatStore::setChatSystemPrompt(qint64 chatId, const QString &systemPrompt)
{
    execute(QStringLiteral("UPDATE chats SET system_prompt = ? WHERE id = ?"), {nonNull(systemPrompt), chatId});
}

void ChatStore::deleteChat(qint64 chatId)
{
    execute(QStringLiteral("DELETE FROM chats WHERE id = ?"), {chatId});
}

qint64 ChatStore::appendMessage(qint64 chatId, const QString &role, const QString &content)
{
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("INSERT INTO messages (chat_id, role, content, created_at) VALUES (?, ?, ?, ?)"));
    query.addBindValue(chatId);
    query.addBindValue(role);
    query.addBindValue(nonNull(content));
    query.addBindValue(m_clock());
    if (!query.exec()) {
        return 0;
    }
    touchChat(chatId);
    return query.lastInsertId().toLongLong();
}

void ChatStore::updateMessageContent(qint64 messageId, const QString &content)
{
    execute(QStringLiteral("UPDATE messages SET content = ? WHERE id = ?"), {nonNull(content), messageId});
    execute(QStringLiteral("UPDATE chats SET updated_at = ? WHERE id = (SELECT chat_id FROM messages WHERE id = ?)"), {m_clock(), messageId});
}

void ChatStore::deleteMessage(qint64 messageId)
{
    execute(QStringLiteral("DELETE FROM messages WHERE id = ?"), {messageId});
}

QList<StoredMessage> ChatStore::messages(qint64 chatId) const
{
    QList<StoredMessage> result;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT id, chat_id, role, content, created_at FROM messages WHERE chat_id = ? ORDER BY id"));
    query.addBindValue(chatId);
    if (query.exec()) {
        while (query.next()) {
            result.append(messageFrom(query));
        }
    }
    return result;
}

std::optional<ChatTranscript> ChatStore::transcript(qint64 chatId) const
{
    const std::optional<ChatSummary> summary = chat(chatId);
    if (!summary) {
        return std::nullopt;
    }
    return ChatTranscript{*summary, messages(chatId)};
}

qint64 ChatStore::importTranscript(const QString &providerId, const ChatTranscript &transcript)
{
    const qint64 chatId = createChat(providerId, transcript.chat.model, transcript.chat.systemPrompt);
    if (chatId == 0) {
        return 0;
    }
    renameChat(chatId, transcript.chat.title);
    for (const StoredMessage &message : transcript.messages) {
        appendMessage(chatId, message.role, message.content);
    }
    return chatId;
}

qint64 ChatStore::currentTimeMs()
{
    return QDateTime::currentMSecsSinceEpoch();
}

QString ChatStore::escapeLikePattern(const QString &text)
{
    QString escaped = text;
    const QString escapeCharacter(LikeEscapeCharacter);
    escaped.replace(escapeCharacter, escapeCharacter + escapeCharacter);
    escaped.replace(QLatin1Char('%'), escapeCharacter + QLatin1Char('%'));
    escaped.replace(QLatin1Char('_'), escapeCharacter + QLatin1Char('_'));
    return escaped;
}

bool ChatStore::openDatabase(const QString &databasePath)
{
    if (databasePath != InMemoryDatabase) {
        QDir().mkpath(QFileInfo(databasePath).absolutePath());
    }
    m_database = QSqlDatabase::addDatabase(QString(DriverName), QUuid::createUuid().toString());
    m_database.setDatabaseName(databasePath);
    return m_database.open();
}

bool ChatStore::createSchema()
{
    return execute(QStringLiteral("PRAGMA foreign_keys = ON"))
        && execute(QStringLiteral("CREATE TABLE IF NOT EXISTS chats ("
                                  "id INTEGER PRIMARY KEY AUTOINCREMENT, provider_id TEXT NOT NULL, title TEXT NOT NULL DEFAULT '', "
                                  "model TEXT NOT NULL DEFAULT '', system_prompt TEXT NOT NULL DEFAULT '', "
                                  "created_at INTEGER NOT NULL, updated_at INTEGER NOT NULL)"))
        && execute(QStringLiteral("CREATE TABLE IF NOT EXISTS messages ("
                                  "id INTEGER PRIMARY KEY AUTOINCREMENT, chat_id INTEGER NOT NULL REFERENCES chats(id) ON DELETE CASCADE, "
                                  "role TEXT NOT NULL, content TEXT NOT NULL, created_at INTEGER NOT NULL)"))
        && execute(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_chats_provider ON chats(provider_id, updated_at DESC)"))
        && execute(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_messages_chat ON messages(chat_id)"));
}

bool ChatStore::execute(const QString &sql, const QVariantList &bindings) const
{
    QSqlQuery query(m_database);
    query.prepare(sql);
    for (const QVariant &binding : bindings) {
        query.addBindValue(binding);
    }
    return query.exec();
}

void ChatStore::touchChat(qint64 chatId)
{
    execute(QStringLiteral("UPDATE chats SET updated_at = ? WHERE id = ?"), {m_clock(), chatId});
}
