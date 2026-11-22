// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ChatExporter.h"
#include "core/ChatMessage.h"
#include "core/ChatStore.h"
#include "core/MarkdownBlocks.h"
#include "core/TitleGenerator.h"

#include <QTemporaryDir>
#include <QTest>

namespace {

const QString ProviderA = QStringLiteral("lmstudio");
const QString ProviderB = QStringLiteral("ollama");
const QString User = QString(ChatRole::User);
const QString Assistant = QString(ChatRole::Assistant);

ChatStore::Clock steppingClock()
{
    auto tick = std::make_shared<qint64>(1000);
    return [tick] { return (*tick)++; };
}

}

class TestChatStore : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void createsAndReadsChat();
    void listsChatsNewestFirstPerProvider();
    void searchesTitlesAndContent();
    void searchTreatsWildcardsLiterally();
    void appendsAndUpdatesMessages();
    void deletingChatRemovesMessages();
    void renamesAndChangesModelAndPrompt();
    void persistsToDiskBetweenInstances();
    void importRecreatesChatWithMessages();
    void exportsMarkdown();
    void jsonRoundTrip();
    void jsonRejectsGarbage();
    void titleComesFromFirstLine();
    void titleIsTruncated();
    void splitsFencedCodeBlocks();
    void keepsUnclosedFenceAsCodeWhileStreaming();
    void plainTextIsSingleBlock();
};

void TestChatStore::createsAndReadsChat()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    QVERIFY(store.isOpen());
    const qint64 id = store.createChat(ProviderA, QStringLiteral("m1"), QStringLiteral("Be brief"));
    QVERIFY(id > 0);
    const std::optional<ChatSummary> chat = store.chat(id);
    QVERIFY(chat.has_value());
    QCOMPARE(chat->providerId, ProviderA);
    QCOMPARE(chat->model, QStringLiteral("m1"));
    QCOMPARE(chat->systemPrompt, QStringLiteral("Be brief"));
    QVERIFY(chat->title.isEmpty());
    QVERIFY(!store.chat(id + 99).has_value());
}

void TestChatStore::listsChatsNewestFirstPerProvider()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    const qint64 older = store.createChat(ProviderA, QString(), QString());
    const qint64 newer = store.createChat(ProviderA, QString(), QString());
    store.createChat(ProviderB, QString(), QString());
    const QList<ChatSummary> chats = store.chats(ProviderA);
    QCOMPARE(chats.size(), 2);
    QCOMPARE(chats.at(0).id, newer);
    QCOMPARE(chats.at(1).id, older);
    store.appendMessage(older, User, QStringLiteral("bump"));
    QCOMPARE(store.chats(ProviderA).first().id, older);
}

void TestChatStore::searchesTitlesAndContent()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    const qint64 byTitle = store.createChat(ProviderA, QString(), QString());
    store.renameChat(byTitle, QStringLiteral("Gardening tips"));
    const qint64 byContent = store.createChat(ProviderA, QString(), QString());
    store.appendMessage(byContent, User, QStringLiteral("How do I prune roses?"));
    store.createChat(ProviderA, QString(), QString());

    QCOMPARE(store.chats(ProviderA, QStringLiteral("garden")).size(), 1);
    QCOMPARE(store.chats(ProviderA, QStringLiteral("PRUNE")).first().id, byContent);
    QCOMPARE(store.chats(ProviderA, QStringLiteral("nothing like this")).size(), 0);
    QCOMPARE(store.chats(ProviderB, QStringLiteral("garden")).size(), 0);
}

void TestChatStore::searchTreatsWildcardsLiterally()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    const qint64 chat = store.createChat(ProviderA, QString(), QString());
    store.appendMessage(chat, User, QStringLiteral("discount is 50% today"));
    store.appendMessage(store.createChat(ProviderA, QString(), QString()), User, QStringLiteral("plain text"));
    QCOMPARE(store.chats(ProviderA, QStringLiteral("50%")).size(), 1);
    QCOMPARE(store.chats(ProviderA, QStringLiteral("%")).size(), 1);
    QCOMPARE(store.chats(ProviderA, QStringLiteral("_")).size(), 0);
}

void TestChatStore::appendsAndUpdatesMessages()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    const qint64 chat = store.createChat(ProviderA, QString(), QString());
    store.appendMessage(chat, User, QStringLiteral("Hi"));
    const qint64 reply = store.appendMessage(chat, Assistant, QString());
    store.updateMessageContent(reply, QStringLiteral("Hello!"));
    const QList<StoredMessage> messages = store.messages(chat);
    QCOMPARE(messages.size(), 2);
    QCOMPARE(messages.at(0).role, User);
    QCOMPARE(messages.at(1).content, QStringLiteral("Hello!"));
    store.deleteMessage(reply);
    QCOMPARE(store.messages(chat).size(), 1);
}

void TestChatStore::deletingChatRemovesMessages()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    const qint64 chat = store.createChat(ProviderA, QString(), QString());
    store.appendMessage(chat, User, QStringLiteral("Hi"));
    store.deleteChat(chat);
    QVERIFY(!store.chat(chat).has_value());
    QVERIFY(store.messages(chat).isEmpty());
}

void TestChatStore::renamesAndChangesModelAndPrompt()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    const qint64 chat = store.createChat(ProviderA, QStringLiteral("old"), QString());
    store.renameChat(chat, QStringLiteral("Title"));
    store.setChatModel(chat, QStringLiteral("new"));
    store.setChatSystemPrompt(chat, QStringLiteral("Prompt"));
    const ChatSummary summary = *store.chat(chat);
    QCOMPARE(summary.title, QStringLiteral("Title"));
    QCOMPARE(summary.model, QStringLiteral("new"));
    QCOMPARE(summary.systemPrompt, QStringLiteral("Prompt"));
}

void TestChatStore::persistsToDiskBetweenInstances()
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("nested/chats.db"));
    {
        ChatStore store(path);
        store.appendMessage(store.createChat(ProviderA, QString(), QString()), User, QStringLiteral("persisted"));
    }
    ChatStore reopened(path);
    QVERIFY(reopened.isOpen());
    const QList<ChatSummary> chats = reopened.chats(ProviderA);
    QCOMPARE(chats.size(), 1);
    QCOMPARE(reopened.messages(chats.first().id).first().content, QStringLiteral("persisted"));
}

void TestChatStore::importRecreatesChatWithMessages()
{
    ChatStore store(QStringLiteral(":memory:"), steppingClock());
    ChatTranscript transcript;
    transcript.chat.title = QStringLiteral("Imported");
    transcript.chat.model = QStringLiteral("m9");
    transcript.messages = {{0, 0, User, QStringLiteral("q"), 0}, {0, 0, Assistant, QStringLiteral("a"), 0}};
    const qint64 id = store.importTranscript(ProviderB, transcript);
    QVERIFY(id > 0);
    QCOMPARE(store.chat(id)->title, QStringLiteral("Imported"));
    QCOMPARE(store.chat(id)->providerId, ProviderB);
    QCOMPARE(store.messages(id).size(), 2);
}

void TestChatStore::exportsMarkdown()
{
    ChatTranscript transcript;
    transcript.chat.title = QStringLiteral("Roses");
    transcript.chat.model = QStringLiteral("m1");
    transcript.messages = {{1, 1, User, QStringLiteral("Prune?"), 0}, {2, 1, Assistant, QStringLiteral("Yes, in spring."), 0}};
    const QString markdown = ChatExporter::toMarkdown(transcript);
    QVERIFY(markdown.startsWith(QStringLiteral("# Roses")));
    QVERIFY(markdown.contains(QStringLiteral("Prune?")));
    QVERIFY(markdown.contains(QStringLiteral("Yes, in spring.")));
    QVERIFY(markdown.indexOf(QStringLiteral("Prune?")) < markdown.indexOf(QStringLiteral("Yes, in spring.")));
}

void TestChatStore::jsonRoundTrip()
{
    ChatTranscript transcript;
    transcript.chat.title = QStringLiteral("Round trip");
    transcript.chat.model = QStringLiteral("m1");
    transcript.chat.systemPrompt = QStringLiteral("Be kind");
    transcript.messages = {{1, 1, User, QStringLiteral("héllo \"quoted\"\nnew line"), 42}};
    const std::optional<ChatTranscript> restored = ChatExporter::fromJson(ChatExporter::toJson(transcript));
    QVERIFY(restored.has_value());
    QCOMPARE(restored->chat.title, transcript.chat.title);
    QCOMPARE(restored->chat.systemPrompt, transcript.chat.systemPrompt);
    QCOMPARE(restored->messages.size(), 1);
    QCOMPARE(restored->messages.first().content, transcript.messages.first().content);
    QCOMPARE(restored->messages.first().createdAt, 42);
}

void TestChatStore::jsonRejectsGarbage()
{
    QVERIFY(!ChatExporter::fromJson("not json").has_value());
    QVERIFY(!ChatExporter::fromJson("{\"title\":\"no messages\"}").has_value());
    const std::optional<ChatTranscript> filtered = ChatExporter::fromJson("{\"messages\":[{\"role\":\"robot\",\"content\":\"x\"},{\"role\":\"user\",\"content\":\"ok\"}]}");
    QVERIFY(filtered.has_value());
    QCOMPARE(filtered->messages.size(), 1);
}

void TestChatStore::titleComesFromFirstLine()
{
    QCOMPARE(TitleGenerator::fromFirstMessage(QStringLiteral("\n\n  ## How do **magnets** work?\nMore detail")), QStringLiteral("How do magnets work?"));
    QVERIFY(TitleGenerator::fromFirstMessage(QStringLiteral("   ")).isEmpty());
}

void TestChatStore::titleIsTruncated()
{
    const QString title = TitleGenerator::fromFirstMessage(QString(200, QLatin1Char('x')));
    QVERIFY(title.size() <= 50);
    QVERIFY(title.endsWith(QStringLiteral("…")));
}

void TestChatStore::splitsFencedCodeBlocks()
{
    const QList<MarkdownBlock> blocks = MarkdownBlocks::split(QStringLiteral("Intro\n\n```python\nprint(1)\n```\nOutro"));
    QCOMPARE(blocks.size(), 3);
    QCOMPARE(blocks.at(0), (MarkdownBlock{MarkdownBlock::Kind::Text, QStringLiteral("Intro"), QString()}));
    QCOMPARE(blocks.at(1), (MarkdownBlock{MarkdownBlock::Kind::Code, QStringLiteral("print(1)"), QStringLiteral("python")}));
    QCOMPARE(blocks.at(2).text, QStringLiteral("Outro"));
}

void TestChatStore::keepsUnclosedFenceAsCodeWhileStreaming()
{
    const QList<MarkdownBlock> blocks = MarkdownBlocks::split(QStringLiteral("Here:\n```js\nconst a = 1;"));
    QCOMPARE(blocks.size(), 2);
    QCOMPARE(blocks.at(1).kind, MarkdownBlock::Kind::Code);
    QCOMPARE(blocks.at(1).text, QStringLiteral("const a = 1;"));
    QCOMPARE(blocks.at(1).language, QStringLiteral("js"));
}

void TestChatStore::plainTextIsSingleBlock()
{
    QCOMPARE(MarkdownBlocks::split(QStringLiteral("just words")).size(), 1);
    QVERIFY(MarkdownBlocks::split(QString()).isEmpty());
}

QTEST_GUILESS_MAIN(TestChatStore)

#include "tst_chatstore.moc"
