// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ChatExporter.h"

#include "core/ChatMessage.h"

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

constexpr QLatin1StringView FormatName{"kompanion-chat"};
constexpr int FormatVersion = 1;
constexpr QLatin1StringView FormatKey{"format"};
constexpr QLatin1StringView VersionKey{"version"};
constexpr QLatin1StringView TitleKey{"title"};
constexpr QLatin1StringView ModelKey{"model"};
constexpr QLatin1StringView SystemPromptKey{"systemPrompt"};
constexpr QLatin1StringView CreatedAtKey{"createdAt"};
constexpr QLatin1StringView UpdatedAtKey{"updatedAt"};
constexpr QLatin1StringView MessagesKey{"messages"};
constexpr QLatin1StringView RoleKey{"role"};
constexpr QLatin1StringView ContentKey{"content"};

QString roleHeading(const QString &role)
{
    if (role == ChatRole::User) {
        return i18n("You");
    }
    if (role == ChatRole::System) {
        return i18n("System");
    }
    return i18n("Assistant");
}

bool isKnownRole(const QString &role)
{
    return role == ChatRole::User || role == ChatRole::Assistant || role == ChatRole::System;
}

QJsonObject messageToJson(const StoredMessage &message)
{
    return QJsonObject{
        {QString(RoleKey), message.role},
        {QString(ContentKey), message.content},
        {QString(CreatedAtKey), message.createdAt},
    };
}

std::optional<StoredMessage> messageFromJson(const QJsonObject &object)
{
    const QString role = object.value(RoleKey).toString();
    if (!isKnownRole(role)) {
        return std::nullopt;
    }
    StoredMessage message;
    message.role = role;
    message.content = object.value(ContentKey).toString();
    message.createdAt = object.value(CreatedAtKey).toInteger();
    return message;
}

}

QString ChatExporter::toMarkdown(const ChatTranscript &transcript)
{
    const QString title = transcript.chat.title.isEmpty() ? i18n("Untitled chat") : transcript.chat.title;
    QString markdown = QStringLiteral("# %1\n\n").arg(title);
    if (!transcript.chat.model.isEmpty()) {
        markdown += i18n("**Model:** %1", transcript.chat.model) + QStringLiteral("\n\n");
    }
    if (!transcript.chat.systemPrompt.isEmpty()) {
        markdown += QStringLiteral("> %1\n\n").arg(i18n("System prompt: %1", transcript.chat.systemPrompt).replace(QLatin1Char('\n'), QStringLiteral("\n> ")));
    }
    for (const StoredMessage &message : transcript.messages) {
        markdown += QStringLiteral("## %1\n\n%2\n\n").arg(roleHeading(message.role), message.content);
    }
    return markdown;
}

QByteArray ChatExporter::toJson(const ChatTranscript &transcript)
{
    QJsonArray messages;
    for (const StoredMessage &message : transcript.messages) {
        messages.append(messageToJson(message));
    }
    const QJsonObject root{
        {QString(FormatKey), QString(FormatName)},
        {QString(VersionKey), FormatVersion},
        {QString(TitleKey), transcript.chat.title},
        {QString(ModelKey), transcript.chat.model},
        {QString(SystemPromptKey), transcript.chat.systemPrompt},
        {QString(CreatedAtKey), transcript.chat.createdAt},
        {QString(UpdatedAtKey), transcript.chat.updatedAt},
        {QString(MessagesKey), messages},
    };
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

std::optional<ChatTranscript> ChatExporter::fromJson(const QByteArray &json)
{
    QJsonParseError parseError;
    const QJsonObject root = QJsonDocument::fromJson(json, &parseError).object();
    if (parseError.error != QJsonParseError::NoError || !root.value(MessagesKey).isArray()) {
        return std::nullopt;
    }
    ChatTranscript transcript;
    transcript.chat.title = root.value(TitleKey).toString();
    transcript.chat.model = root.value(ModelKey).toString();
    transcript.chat.systemPrompt = root.value(SystemPromptKey).toString();
    transcript.chat.createdAt = root.value(CreatedAtKey).toInteger();
    transcript.chat.updatedAt = root.value(UpdatedAtKey).toInteger();
    const QJsonArray messages = root.value(MessagesKey).toArray();
    for (const QJsonValue &value : messages) {
        if (const std::optional<StoredMessage> message = messageFromJson(value.toObject())) {
            transcript.messages.append(*message);
        }
    }
    return transcript;
}
