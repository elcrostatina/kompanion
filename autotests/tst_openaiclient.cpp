// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ApiUrl.h"
#include "core/OpenAiClient.h"
#include "core/OpenAiProtocol.h"
#include "core/SseParser.h"

#include "MockServer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QTimer>

#include <functional>

namespace {

const QList<ChatMessage> sampleConversation{{QString(ChatRole::User), QStringLiteral("Hi")}};

}

class TestOpenAiClient : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void sseParserJoinsEventsSplitAcrossChunks();
    void sseParserHandlesCrLfAndIgnoresOtherFields();
    void sseParserFinishFlushesUnterminatedEvent();
    void apiUrlAppendsVersionWhenMissing();
    void apiUrlKeepsExistingPath();
    void protocolParsesDeltaDoneAndErrors();
    void requestBodyContainsModelMessagesAndStreamFlag();
    void streamsDeltasUntilDoneMarker();
    void sendsBearerTokenAndJsonBody();
    void reportsUnreachableServer();
    void reportsMissingModel();
    void reportsRejectedApiKey();
    void listsModels();
    void cancelFinishesWithPartialText();
};

void TestOpenAiClient::sseParserJoinsEventsSplitAcrossChunks()
{
    SseParser parser;
    QVERIFY(parser.feed("data: {\"a\":").isEmpty());
    const QList<QByteArray> events = parser.feed("1}\n\ndata: second\n\n");
    QCOMPARE(events, (QList<QByteArray>{"{\"a\":1}", "second"}));
}

void TestOpenAiClient::sseParserHandlesCrLfAndIgnoresOtherFields()
{
    SseParser parser;
    const QList<QByteArray> events = parser.feed(": comment\r\nevent: message\r\ndata: one\r\ndata: two\r\n\r\n");
    QCOMPARE(events, (QList<QByteArray>{"one\ntwo"}));
}

void TestOpenAiClient::sseParserFinishFlushesUnterminatedEvent()
{
    SseParser parser;
    QVERIFY(parser.feed("data: tail").isEmpty());
    QCOMPARE(parser.finish(), (QList<QByteArray>{"tail"}));
}

void TestOpenAiClient::apiUrlAppendsVersionWhenMissing()
{
    QCOMPARE(ApiUrl::endpoint(QUrl(QStringLiteral("http://localhost:1234")), QStringLiteral("models")).toString(), QStringLiteral("http://localhost:1234/v1/models"));
}

void TestOpenAiClient::apiUrlKeepsExistingPath()
{
    QCOMPARE(ApiUrl::endpoint(QUrl(QStringLiteral("http://lan:8080/api/v1/")), QStringLiteral("chat/completions")).toString(), QStringLiteral("http://lan:8080/api/v1/chat/completions"));
}

void TestOpenAiClient::protocolParsesDeltaDoneAndErrors()
{
    using Kind = OpenAiProtocol::StreamEvent::Kind;
    QCOMPARE(OpenAiProtocol::parseStreamEvent("[DONE]").kind, Kind::Done);
    QCOMPARE(OpenAiProtocol::parseStreamEvent("{\"choices\":[{\"delta\":{\"role\":\"assistant\"}}]}").kind, Kind::Ignored);
    const OpenAiProtocol::StreamEvent delta = OpenAiProtocol::parseStreamEvent("{\"choices\":[{\"delta\":{\"content\":\"Hey\"}}]}");
    QCOMPARE(delta.kind, Kind::Delta);
    QCOMPARE(delta.text, QStringLiteral("Hey"));
    const OpenAiProtocol::StreamEvent failure = OpenAiProtocol::parseStreamEvent("{\"error\":{\"message\":\"boom\"}}");
    QCOMPARE(failure.kind, Kind::Failure);
    QCOMPARE(failure.text, QStringLiteral("boom"));
}

void TestOpenAiClient::requestBodyContainsModelMessagesAndStreamFlag()
{
    const QJsonObject body = QJsonDocument::fromJson(OpenAiProtocol::chatRequestBody(QStringLiteral("m1"), sampleConversation)).object();
    QCOMPARE(body.value(QStringLiteral("model")).toString(), QStringLiteral("m1"));
    QVERIFY(body.value(QStringLiteral("stream")).toBool());
    const QJsonObject message = body.value(QStringLiteral("messages")).toArray().first().toObject();
    QCOMPARE(message.value(QStringLiteral("role")).toString(), QStringLiteral("user"));
    QCOMPARE(message.value(QStringLiteral("content")).toString(), QStringLiteral("Hi"));
}

void TestOpenAiClient::streamsDeltasUntilDoneMarker()
{
    MockServer server([](QTcpSocket *socket) {
        const QByteArray second = deltaEvent(QStringLiteral("lo"));
        socket->write(eventStreamHeaders() + deltaEvent(QStringLiteral("Hel")) + second.left(10));
        writeAfterPause(socket, second.mid(10) + "data: [DONE]\n\n", true);
    });
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(server.baseUrl(), QString());
    QSignalSpy deltas(&client, &OpenAiClient::chatDeltaReceived);
    QSignalSpy finished(&client, &OpenAiClient::chatFinished);

    client.streamChat(QStringLiteral("m1"), sampleConversation);

    QVERIFY(finished.wait(SignalTimeoutMs));
    QCOMPARE(deltas.size(), 2);
    QCOMPARE(deltas.at(0).at(0).toString(), QStringLiteral("Hel"));
    QCOMPARE(deltas.at(1).at(0).toString(), QStringLiteral("lo"));
    QVERIFY(!client.isStreaming());
}

void TestOpenAiClient::sendsBearerTokenAndJsonBody()
{
    MockServer server([](QTcpSocket *socket) {
        socket->write(eventStreamHeaders() + "data: [DONE]\n\n");
        socket->disconnectFromHost();
    });
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(server.baseUrl(), QStringLiteral("secret-key"));
    QSignalSpy finished(&client, &OpenAiClient::chatFinished);

    client.streamChat(QStringLiteral("m1"), sampleConversation);

    QVERIFY(finished.wait(SignalTimeoutMs));
    const QByteArray request = server.lastRequest();
    QVERIFY(request.startsWith("POST /v1/chat/completions"));
    QVERIFY(request.toLower().contains("authorization: bearer secret-key"));
    QVERIFY(request.contains("\"model\":\"m1\""));
}

void TestOpenAiClient::reportsUnreachableServer()
{
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(QUrl(QStringLiteral("http://127.0.0.1:%1/v1").arg(UnusedPort)), QString());
    QSignalSpy failed(&client, &OpenAiClient::chatFailed);

    client.streamChat(QStringLiteral("m1"), sampleConversation);

    QVERIFY(failed.wait(SignalTimeoutMs));
    QCOMPARE(failed.at(0).at(0).value<ClientError>().kind, ClientError::Kind::ServerUnreachable);
}

void TestOpenAiClient::reportsMissingModel()
{
    MockServer server([](QTcpSocket *socket) {
        respondWithJson(socket, 400, "{\"error\":{\"message\":\"No models loaded. Load a model first.\"}}");
    });
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(server.baseUrl(), QString());
    QSignalSpy failed(&client, &OpenAiClient::chatFailed);

    client.streamChat(QStringLiteral("m1"), sampleConversation);

    QVERIFY(failed.wait(SignalTimeoutMs));
    const ClientError error = failed.at(0).at(0).value<ClientError>();
    QCOMPARE(error.kind, ClientError::Kind::NoModelLoaded);
    QVERIFY(error.message.contains(QStringLiteral("No models loaded")));
}

void TestOpenAiClient::reportsRejectedApiKey()
{
    MockServer server([](QTcpSocket *socket) {
        respondWithJson(socket, 401, "{\"error\":{\"message\":\"bad key\"}}");
    });
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(server.baseUrl(), QStringLiteral("wrong"));
    QSignalSpy failed(&client, &OpenAiClient::modelsFailed);

    client.listModels();

    QVERIFY(failed.wait(SignalTimeoutMs));
    QCOMPARE(failed.at(0).at(0).value<ClientError>().kind, ClientError::Kind::Unauthorized);
}

void TestOpenAiClient::listsModels()
{
    MockServer server([](QTcpSocket *socket) {
        respondWithJson(socket, 200, "{\"data\":[{\"id\":\"alpha\"},{\"id\":\"beta\"}]}");
    });
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(server.baseUrl(), QString());
    QSignalSpy received(&client, &OpenAiClient::modelsReceived);

    client.listModels();

    QVERIFY(received.wait(SignalTimeoutMs));
    QCOMPARE(received.at(0).at(0).toStringList(), (QStringList{QStringLiteral("alpha"), QStringLiteral("beta")}));
}

void TestOpenAiClient::cancelFinishesWithPartialText()
{
    MockServer server([](QTcpSocket *socket) {
        socket->write(eventStreamHeaders() + deltaEvent(QStringLiteral("partial")));
    });
    QNetworkAccessManager network;
    OpenAiClient client(network);
    client.setEndpoint(server.baseUrl(), QString());
    QSignalSpy deltas(&client, &OpenAiClient::chatDeltaReceived);
    QSignalSpy finished(&client, &OpenAiClient::chatFinished);

    client.streamChat(QStringLiteral("m1"), sampleConversation);
    QVERIFY(deltas.wait(SignalTimeoutMs));
    client.cancelChat();

    QCOMPARE(finished.size(), 1);
    QVERIFY(!client.isStreaming());
}

QTEST_GUILESS_MAIN(TestOpenAiClient)

#include "tst_openaiclient.moc"
