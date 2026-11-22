// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ChatMessage.h"
#include "core/ClientError.h"
#include "core/SseParser.h"

#include <QList>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

#include <functional>

class QNetworkAccessManager;
class QNetworkReply;

class OpenAiClient : public QObject
{
    Q_OBJECT

public:
    explicit OpenAiClient(QNetworkAccessManager &network, QObject *parent = nullptr);

    void setEndpoint(const QUrl &baseUrl, const QString &apiKey);

    void listModels();
    void streamChat(const QString &model, const QList<ChatMessage> &messages);
    void cancelChat();
    bool isStreaming() const;

Q_SIGNALS:
    void modelsReceived(const QStringList &modelIds);
    void modelsFailed(const ClientError &error);
    void chatDeltaReceived(const QString &text);
    void chatFinished();
    void chatFailed(const ClientError &error);

private:
    QNetworkRequest makeRequest(const QString &relativePath) const;
    ClientError classifyFailure(QNetworkReply *reply, const QByteArray &body) const;
    void handleModelsFinished(QNetworkReply *reply);
    void handleChatReadyRead();
    void handleChatFinished();
    void dispatchStreamEvents(const QList<QByteArray> &events);
    void concludeStream(const std::function<void()> &notify);

    QNetworkAccessManager &m_network;
    QUrl m_baseUrl;
    QString m_apiKey;
    QNetworkReply *m_chatReply = nullptr;
    SseParser m_parser;
    QByteArray m_errorBody;
};
