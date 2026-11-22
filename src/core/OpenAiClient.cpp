// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/OpenAiClient.h"

#include "core/ApiUrl.h"
#include "core/OpenAiProtocol.h"

#include <KLocalizedString>

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace {

constexpr int HttpOk = 200;
constexpr int HttpBadRequest = 400;
constexpr int HttpUnauthorized = 401;
constexpr int HttpForbidden = 403;
constexpr int HttpNotFound = 404;
constexpr int ModelsRequestTimeoutMs = 15000;
constexpr QLatin1StringView ModelsPath{"models"};
constexpr QLatin1StringView ChatCompletionsPath{"chat/completions"};
constexpr QLatin1StringView JsonContentType{"application/json"};
constexpr QLatin1StringView EventStreamContentType{"text/event-stream"};
constexpr QLatin1StringView ModelKeyword{"model"};

int httpStatusOf(const QNetworkReply *reply)
{
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

bool mentionsModel(const QString &serverMessage)
{
    return serverMessage.contains(ModelKeyword, Qt::CaseInsensitive);
}

bool isMissingModelResponse(int status, const QString &serverMessage)
{
    const bool isClientFault = status == HttpBadRequest || status == HttpNotFound;
    return isClientFault && mentionsModel(serverMessage);
}

QString serverDescription(const QUrl &baseUrl)
{
    return baseUrl.port() > 0 ? QStringLiteral("%1:%2").arg(baseUrl.host()).arg(baseUrl.port()) : baseUrl.host();
}

ClientError unreachableError(const QUrl &baseUrl)
{
    return {ClientError::Kind::ServerUnreachable,
            i18n("Cannot reach the server at %1. Check that it is running and that the address is correct.", serverDescription(baseUrl)),
            0};
}

ClientError serverError(int status, const QString &serverMessage)
{
    const QString message = serverMessage.isEmpty() ? i18n("The server returned an error (HTTP %1).", status)
                                                    : i18n("The server returned an error (HTTP %1): %2", status, serverMessage);
    return {ClientError::Kind::ServerError, message, status};
}

}

OpenAiClient::OpenAiClient(QNetworkAccessManager &network, QObject *parent)
    : QObject(parent)
    , m_network(network)
{
}

void OpenAiClient::setEndpoint(const QUrl &baseUrl, const QString &apiKey)
{
    m_baseUrl = baseUrl;
    m_apiKey = apiKey;
}

void OpenAiClient::listModels()
{
    QNetworkRequest request = makeRequest(QString(ModelsPath));
    request.setTransferTimeout(ModelsRequestTimeoutMs);
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        handleModelsFinished(reply);
    });
}

void OpenAiClient::streamChat(const QString &model, const QList<ChatMessage> &messages)
{
    if (m_chatReply) {
        return;
    }
    m_parser = SseParser();
    m_errorBody.clear();
    QNetworkRequest request = makeRequest(QString(ChatCompletionsPath));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString(JsonContentType));
    request.setRawHeader("Accept", QByteArray(EventStreamContentType.data()));
    m_chatReply = m_network.post(request, OpenAiProtocol::chatRequestBody(model, messages));
    connect(m_chatReply, &QNetworkReply::readyRead, this, &OpenAiClient::handleChatReadyRead);
    connect(m_chatReply, &QNetworkReply::finished, this, &OpenAiClient::handleChatFinished);
}

void OpenAiClient::cancelChat()
{
    if (m_chatReply) {
        concludeStream([this] { Q_EMIT chatFinished(); });
    }
}

bool OpenAiClient::isStreaming() const
{
    return m_chatReply != nullptr;
}

QNetworkRequest OpenAiClient::makeRequest(const QString &relativePath) const
{
    QNetworkRequest request(ApiUrl::endpoint(m_baseUrl, relativePath));
    if (!m_apiKey.isEmpty()) {
        request.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + m_apiKey.toUtf8());
    }
    return request;
}

ClientError OpenAiClient::classifyFailure(QNetworkReply *reply, const QByteArray &body) const
{
    const int status = httpStatusOf(reply);
    if (status == 0) {
        return unreachableError(m_baseUrl);
    }
    const QString serverMessage = OpenAiProtocol::parseErrorMessage(body);
    if (status == HttpUnauthorized || status == HttpForbidden) {
        return {ClientError::Kind::Unauthorized, i18n("The server rejected the API key (HTTP %1).", status), status};
    }
    if (isMissingModelResponse(status, serverMessage)) {
        return {ClientError::Kind::NoModelLoaded, i18n("No usable model: %1", serverMessage), status};
    }
    return serverError(status, serverMessage);
}

void OpenAiClient::handleModelsFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    const QByteArray body = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT modelsFailed(classifyFailure(reply, body));
        return;
    }
    const bool hasModelList = QJsonDocument::fromJson(body).object().contains(QStringLiteral("data"));
    if (!hasModelList) {
        Q_EMIT modelsFailed({ClientError::Kind::InvalidResponse, i18n("The server did not return a valid model list."), httpStatusOf(reply)});
        return;
    }
    Q_EMIT modelsReceived(OpenAiProtocol::parseModelIds(body));
}

void OpenAiClient::handleChatReadyRead()
{
    const QByteArray data = m_chatReply->readAll();
    const int status = httpStatusOf(m_chatReply);
    if (status != HttpOk) {
        m_errorBody.append(data);
        return;
    }
    dispatchStreamEvents(m_parser.feed(data));
}

void OpenAiClient::handleChatFinished()
{
    QNetworkReply *reply = m_chatReply;
    if (reply->error() != QNetworkReply::NoError) {
        const ClientError error = classifyFailure(reply, m_errorBody + reply->readAll());
        concludeStream([this, error] { Q_EMIT chatFailed(error); });
        return;
    }
    dispatchStreamEvents(m_parser.feed(reply->readAll()));
    dispatchStreamEvents(m_parser.finish());
    if (m_chatReply) {
        concludeStream([this] { Q_EMIT chatFinished(); });
    }
}

void OpenAiClient::dispatchStreamEvents(const QList<QByteArray> &events)
{
    for (const QByteArray &eventData : events) {
        if (!m_chatReply) {
            return;
        }
        const OpenAiProtocol::StreamEvent event = OpenAiProtocol::parseStreamEvent(eventData);
        if (event.kind == OpenAiProtocol::StreamEvent::Kind::Delta) {
            Q_EMIT chatDeltaReceived(event.text);
        } else if (event.kind == OpenAiProtocol::StreamEvent::Kind::Done) {
            concludeStream([this] { Q_EMIT chatFinished(); });
        } else if (event.kind == OpenAiProtocol::StreamEvent::Kind::Failure) {
            const ClientError error = serverError(HttpOk, event.text);
            concludeStream([this, error] { Q_EMIT chatFailed(error); });
        }
    }
}

void OpenAiClient::concludeStream(const std::function<void()> &notify)
{
    QNetworkReply *reply = std::exchange(m_chatReply, nullptr);
    if (reply) {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
    notify();
}
