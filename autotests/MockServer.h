// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

#include <functional>

namespace MockServerSupport {

constexpr int SignalTimeoutMs = 5000;
constexpr int ChunkPauseMs = 40;
constexpr quint16 UnusedPort = 1;

class MockServer : public QObject
{
public:
    using Responder = std::function<void(QTcpSocket *)>;

    explicit MockServer(Responder responder)
        : m_responder(std::move(responder))
    {
        connect(&m_server, &QTcpServer::newConnection, this, &MockServer::acceptConnection);
        m_server.listen(QHostAddress::LocalHost);
    }

    QUrl baseUrl() const
    {
        return QUrl(QStringLiteral("http://127.0.0.1:%1/v1").arg(m_server.serverPort()));
    }

    QByteArray lastRequest() const
    {
        return m_lastRequest;
    }

private:
    void acceptConnection()
    {
        QTcpSocket *socket = m_server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
            m_pending[socket].append(socket->readAll());
            if (isComplete(m_pending[socket])) {
                m_lastRequest = m_pending.take(socket);
                m_responder(socket);
            }
        });
    }

    static bool isComplete(const QByteArray &request)
    {
        const qsizetype headerEnd = request.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            return false;
        }
        const qsizetype lengthPosition = request.toLower().indexOf("content-length:");
        const qsizetype bodyLength = lengthPosition < 0 ? 0 : request.mid(lengthPosition + 15, request.indexOf("\r\n", lengthPosition) - lengthPosition - 15).trimmed().toInt();
        return request.size() >= headerEnd + 4 + bodyLength;
    }

    QTcpServer m_server;
    Responder m_responder;
    QHash<QTcpSocket *, QByteArray> m_pending;
    QByteArray m_lastRequest;
};

inline QByteArray eventStreamHeaders()
{
    return QByteArrayLiteral("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nConnection: close\r\n\r\n");
}

inline QByteArray deltaEvent(const QString &text)
{
    const QJsonObject chunk{{QStringLiteral("choices"), QJsonArray{QJsonObject{{QStringLiteral("delta"), QJsonObject{{QStringLiteral("content"), text}}}}}}};
    return "data: " + QJsonDocument(chunk).toJson(QJsonDocument::Compact) + "\n\n";
}

inline void respondWithJson(QTcpSocket *socket, int status, const QByteArray &json)
{
    const QByteArray head = "HTTP/1.1 " + QByteArray::number(status) + " Status\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length: "
        + QByteArray::number(json.size()) + "\r\n\r\n";
    socket->write(head + json);
    socket->disconnectFromHost();
}

inline void writeAfterPause(QTcpSocket *socket, const QByteArray &data, bool closeAfterwards)
{
    QTimer::singleShot(ChunkPauseMs, socket, [socket, data, closeAfterwards] {
        socket->write(data);
        if (closeAfterwards) {
            socket->disconnectFromHost();
        }
    });
}

}

using namespace MockServerSupport;
