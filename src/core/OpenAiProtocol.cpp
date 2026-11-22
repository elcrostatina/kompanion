// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/OpenAiProtocol.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

const QByteArray DoneMarker = QByteArrayLiteral("[DONE]");

QJsonObject messageToJson(const ChatMessage &message)
{
    return QJsonObject{{QStringLiteral("role"), message.role}, {QStringLiteral("content"), message.content}};
}

QString deltaContent(const QJsonObject &chunk)
{
    const QJsonArray choices = chunk.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return QString();
    }
    const QJsonObject delta = choices.first().toObject().value(QStringLiteral("delta")).toObject();
    return delta.value(QStringLiteral("content")).toString();
}

QString errorMessageFrom(const QJsonObject &root)
{
    const QJsonValue error = root.value(QStringLiteral("error"));
    if (error.isString()) {
        return error.toString();
    }
    return error.toObject().value(QStringLiteral("message")).toString();
}

}

QByteArray OpenAiProtocol::chatRequestBody(const QString &model, const QList<ChatMessage> &messages)
{
    QJsonArray jsonMessages;
    for (const ChatMessage &message : messages) {
        jsonMessages.append(messageToJson(message));
    }
    const QJsonObject body{
        {QStringLiteral("model"), model},
        {QStringLiteral("messages"), jsonMessages},
        {QStringLiteral("stream"), true},
    };
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

OpenAiProtocol::StreamEvent OpenAiProtocol::parseStreamEvent(const QByteArray &eventData)
{
    if (eventData.trimmed() == DoneMarker) {
        return {StreamEvent::Kind::Done, QString()};
    }
    const QJsonObject chunk = QJsonDocument::fromJson(eventData).object();
    const QString failure = errorMessageFrom(chunk);
    if (!failure.isEmpty()) {
        return {StreamEvent::Kind::Failure, failure};
    }
    const QString content = deltaContent(chunk);
    if (content.isEmpty()) {
        return {StreamEvent::Kind::Ignored, QString()};
    }
    return {StreamEvent::Kind::Delta, content};
}

QStringList OpenAiProtocol::parseModelIds(const QByteArray &responseBody)
{
    QStringList ids;
    const QJsonArray models = QJsonDocument::fromJson(responseBody).object().value(QStringLiteral("data")).toArray();
    for (const QJsonValue &model : models) {
        const QString id = model.toObject().value(QStringLiteral("id")).toString();
        if (!id.isEmpty()) {
            ids.append(id);
        }
    }
    return ids;
}

QString OpenAiProtocol::parseErrorMessage(const QByteArray &responseBody)
{
    return errorMessageFrom(QJsonDocument::fromJson(responseBody).object());
}
