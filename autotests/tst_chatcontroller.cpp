// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ChatController.h"
#include "core/AppSettings.h"
#include "core/ChatStore.h"
#include "core/InMemorySecretStore.h"
#include "core/ProviderRegistry.h"

#include "MockServer.h"

#include <QNetworkAccessManager>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace {

const QString ProviderId = QStringLiteral("lmstudio");
const QString ModelName = QStringLiteral("test-model");

}

class TestChatController : public QObject
{
    Q_OBJECT

private:
    struct Fixture {
        QTemporaryDir directory;
        KSharedConfigPtr config;
        std::unique_ptr<AppSettings> settings;
        std::unique_ptr<ProviderRegistry> registry;
        std::unique_ptr<ChatStore> store;
        InMemorySecretStore secrets;
        QNetworkAccessManager network;
        std::unique_ptr<ChatController> controller;

        explicit Fixture(const QUrl &serverUrl)
        {
            config = KSharedConfig::openConfig(directory.filePath(QStringLiteral("rc")), KConfig::SimpleConfig);
            config->reparseConfiguration();
            settings = std::make_unique<AppSettings>(config);
            registry = std::make_unique<ProviderRegistry>(config);
            registry->seedMissingPresets();
            Provider provider = *registry->find(ProviderId);
            provider.url = serverUrl;
            provider.enabled = true;
            provider.defaultModel = ModelName;
            registry->update(provider);
            store = std::make_unique<ChatStore>(QStringLiteral(":memory:"));
            controller = std::make_unique<ChatController>(ProviderId, *store, *registry, *settings, secrets, network);
        }
    };

    static void respondWithReply(QTcpSocket *socket, const QStringList &deltas)
    {
        QByteArray body = eventStreamHeaders();
        for (const QString &delta : deltas) {
            body += deltaEvent(delta);
        }
        socket->write(body + "data: [DONE]\n\n");
        socket->disconnectFromHost();
    }

private Q_SLOTS:
    void sendStreamsAndPersistsConversation();
    void sendIncludesSystemPromptAndHistory();
    void unreachableServerShowsErrorAndKeepsUserMessage();
    void sendWithoutModelShowsErrorAndCreatesNoChat();
    void refreshModelsSelectsDefaultOrFirst();
    void emptyModelListExplainsNoModelLoaded();
    void stopKeepsPartialAnswer();
    void deleteCurrentChatReturnsToDraft();
    void reopensMostRecentChatOnStart();
    void searchFiltersChatList();
    void importAndExportRoundTrip();
};

void TestChatController::sendStreamsAndPersistsConversation()
{
    MockServer server([](QTcpSocket *socket) { respondWithReply(socket, {QStringLiteral("Hello "), QStringLiteral("world")}); });
    Fixture fixture(server.baseUrl());
    QSignalSpy busyChanges(fixture.controller.get(), &ChatController::busyChanged);

    fixture.controller->sendMessage(QStringLiteral("  Say hello please  "));
    QVERIFY(fixture.controller->isBusy());
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->isBusy(), SignalTimeoutMs);

    const qlonglong chatId = fixture.controller->currentChatId();
    QVERIFY(chatId > 0);
    QCOMPARE(fixture.controller->currentTitle(), QStringLiteral("Say hello please"));
    const QList<StoredMessage> stored = fixture.store->messages(chatId);
    QCOMPARE(stored.size(), 2);
    QCOMPARE(stored.at(0).content, QStringLiteral("Say hello please"));
    QCOMPARE(stored.at(1).role, QStringLiteral("assistant"));
    QCOMPARE(stored.at(1).content, QStringLiteral("Hello world"));
    QCOMPARE(fixture.controller->messages()->rowCount(), 2);
    QCOMPARE(fixture.controller->chats()->rowCount(), 1);
    QVERIFY(fixture.controller->errorText().isEmpty());
}

void TestChatController::sendIncludesSystemPromptAndHistory()
{
    MockServer server([](QTcpSocket *socket) { respondWithReply(socket, {QStringLiteral("ok")}); });
    Fixture fixture(server.baseUrl());
    fixture.controller->changeSystemPrompt(QStringLiteral("Answer in French"));

    fixture.controller->sendMessage(QStringLiteral("first question"));
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->isBusy(), SignalTimeoutMs);
    fixture.controller->sendMessage(QStringLiteral("second question"));
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->isBusy(), SignalTimeoutMs);

    const QJsonObject body = QJsonDocument::fromJson(server.lastRequest().mid(server.lastRequest().indexOf("\r\n\r\n") + 4)).object();
    QCOMPARE(body.value(QStringLiteral("model")).toString(), ModelName);
    const QJsonArray messages = body.value(QStringLiteral("messages")).toArray();
    QCOMPARE(messages.size(), 4);
    QCOMPARE(messages.at(0).toObject().value(QStringLiteral("role")).toString(), QStringLiteral("system"));
    QCOMPARE(messages.at(0).toObject().value(QStringLiteral("content")).toString(), QStringLiteral("Answer in French"));
    QCOMPARE(messages.at(1).toObject().value(QStringLiteral("content")).toString(), QStringLiteral("first question"));
    QCOMPARE(messages.at(2).toObject().value(QStringLiteral("role")).toString(), QStringLiteral("assistant"));
    QCOMPARE(messages.at(3).toObject().value(QStringLiteral("content")).toString(), QStringLiteral("second question"));
}

void TestChatController::unreachableServerShowsErrorAndKeepsUserMessage()
{
    Fixture fixture(QUrl(QStringLiteral("http://127.0.0.1:%1/v1").arg(UnusedPort)));
    fixture.controller->sendMessage(QStringLiteral("anyone there?"));
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->isBusy(), SignalTimeoutMs);

    QVERIFY(fixture.controller->errorText().contains(QStringLiteral("Cannot reach")));
    const QList<StoredMessage> stored = fixture.store->messages(fixture.controller->currentChatId());
    QCOMPARE(stored.size(), 1);
    QCOMPARE(stored.first().role, QStringLiteral("user"));
    QCOMPARE(fixture.controller->messages()->rowCount(), 1);
}

void TestChatController::sendWithoutModelShowsErrorAndCreatesNoChat()
{
    Fixture fixture(QUrl(QStringLiteral("http://127.0.0.1:%1/v1").arg(UnusedPort)));
    fixture.controller->selectModel(QString());
    fixture.controller->sendMessage(QStringLiteral("hello"));
    QVERIFY(!fixture.controller->errorText().isEmpty());
    QCOMPARE(fixture.controller->currentChatId(), 0);
    QVERIFY(!fixture.controller->isBusy());
}

void TestChatController::refreshModelsSelectsDefaultOrFirst()
{
    MockServer server([](QTcpSocket *socket) { respondWithJson(socket, 200, "{\"data\":[{\"id\":\"alpha\"},{\"id\":\"test-model\"}]}"); });
    Fixture fixture(server.baseUrl());
    fixture.controller->refreshModels();
    QTRY_COMPARE_WITH_TIMEOUT(fixture.controller->models().size(), 2, SignalTimeoutMs);
    QCOMPARE(fixture.controller->currentModel(), ModelName);

    fixture.controller->selectModel(QStringLiteral("alpha"));
    QCOMPARE(fixture.registry->find(ProviderId)->defaultModel, QStringLiteral("alpha"));
}

void TestChatController::emptyModelListExplainsNoModelLoaded()
{
    MockServer server([](QTcpSocket *socket) { respondWithJson(socket, 200, "{\"data\":[]}"); });
    Fixture fixture(server.baseUrl());
    fixture.controller->refreshModels();
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->errorText().isEmpty(), SignalTimeoutMs);
    QVERIFY(fixture.controller->errorText().contains(QStringLiteral("No model is loaded")));
}

void TestChatController::stopKeepsPartialAnswer()
{
    MockServer server([](QTcpSocket *socket) { socket->write(eventStreamHeaders() + deltaEvent(QStringLiteral("partial answer"))); });
    Fixture fixture(server.baseUrl());
    fixture.controller->sendMessage(QStringLiteral("long question"));
    QTRY_COMPARE_WITH_TIMEOUT(fixture.controller->messages()->rowCount(), 2, SignalTimeoutMs);
    QTRY_VERIFY_WITH_TIMEOUT(fixture.controller->messages()->data(fixture.controller->messages()->index(1), MessageModel::BlockTextRole).toString() == QStringLiteral("partial answer"), SignalTimeoutMs);

    fixture.controller->stop();

    QVERIFY(!fixture.controller->isBusy());
    QCOMPARE(fixture.store->messages(fixture.controller->currentChatId()).last().content, QStringLiteral("partial answer"));
}

void TestChatController::deleteCurrentChatReturnsToDraft()
{
    MockServer server([](QTcpSocket *socket) { respondWithReply(socket, {QStringLiteral("ok")}); });
    Fixture fixture(server.baseUrl());
    fixture.controller->sendMessage(QStringLiteral("hi"));
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->isBusy(), SignalTimeoutMs);

    fixture.controller->deleteChat(fixture.controller->currentChatId());

    QCOMPARE(fixture.controller->currentChatId(), 0);
    QCOMPARE(fixture.controller->messages()->rowCount(), 0);
    QCOMPARE(fixture.controller->chats()->rowCount(), 0);
}

void TestChatController::reopensMostRecentChatOnStart()
{
    Fixture fixture(QUrl(QStringLiteral("http://127.0.0.1:%1/v1").arg(UnusedPort)));
    const qint64 older = fixture.store->createChat(ProviderId, QStringLiteral("m"), QString());
    fixture.store->appendMessage(older, QStringLiteral("user"), QStringLiteral("old"));
    const qint64 newer = fixture.store->createChat(ProviderId, QStringLiteral("m"), QString());
    fixture.store->appendMessage(newer, QStringLiteral("user"), QStringLiteral("new"));
    ChatController reopened(ProviderId, *fixture.store, *fixture.registry, *fixture.settings, fixture.secrets, fixture.network);
    QCOMPARE(reopened.currentChatId(), newer);
    QCOMPARE(reopened.messages()->rowCount(), 1);
}

void TestChatController::searchFiltersChatList()
{
    Fixture fixture(QUrl(QStringLiteral("http://127.0.0.1:%1/v1").arg(UnusedPort)));
    const qint64 first = fixture.store->createChat(ProviderId, QString(), QString());
    fixture.store->renameChat(first, QStringLiteral("Cooking pasta"));
    const qint64 second = fixture.store->createChat(ProviderId, QString(), QString());
    fixture.store->renameChat(second, QStringLiteral("Rust lifetimes"));
    fixture.controller->search(QStringLiteral("pasta"));
    QCOMPARE(fixture.controller->chats()->rowCount(), 1);
    fixture.controller->search(QString());
    QCOMPARE(fixture.controller->chats()->rowCount(), 2);
}

void TestChatController::importAndExportRoundTrip()
{
    MockServer server([](QTcpSocket *socket) { respondWithReply(socket, {QStringLiteral("answer")}); });
    Fixture fixture(server.baseUrl());
    fixture.controller->sendMessage(QStringLiteral("question"));
    QTRY_VERIFY_WITH_TIMEOUT(!fixture.controller->isBusy(), SignalTimeoutMs);
    const qlonglong original = fixture.controller->currentChatId();
    const QUrl target = QUrl::fromLocalFile(fixture.directory.filePath(QStringLiteral("export.json")));

    fixture.controller->exportChat(original, QStringLiteral("json"), target);
    fixture.controller->importChat(target);

    QVERIFY(fixture.controller->currentChatId() != original);
    QCOMPARE(fixture.controller->messages()->rowCount(), 2);
    QCOMPARE(fixture.controller->chats()->rowCount(), 2);
    QVERIFY(fixture.controller->errorText().isEmpty());
}

QTEST_MAIN(TestChatController)

#include "tst_chatcontroller.moc"
