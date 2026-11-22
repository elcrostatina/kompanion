// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/FaviconStore.h"

#include <QBuffer>
#include <QGuiApplication>
#include "MockServer.h"

#include <QImage>
#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace {

class FakeFaviconProvider : public QQuickImageProvider
{
public:
    FakeFaviconProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &) override
    {
        requestedIds.append(id);
        QImage image(16, 16, QImage::Format_ARGB32);
        image.fill(id.contains(QStringLiteral("other")) ? Qt::blue : Qt::red);
        *size = image.size();
        return image;
    }

    QStringList requestedIds;
};

}

class TestFaviconStore : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void capturesIconFromTheFaviconProvider();
    void unchangedIconDoesNotBumpTheRevision();
    void ignoresOtherSchemesAndHosts();
    void unknownProviderHasNoUrl();
    void removeDeletesTheFile();
    void fetchesFaviconIcoWhenMissing();
    void doesNotFetchWhenAlreadyStored();
    void ignoresNonWebAddresses();
};

void TestFaviconStore::capturesIconFromTheFaviconProvider()
{
    QQmlEngine engine;
    auto *provider = new FakeFaviconProvider;
    engine.addImageProvider(QStringLiteral("favicon"), provider);
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.filePath(QStringLiteral("icons")));
    QSignalSpy changes(&store, &FaviconStore::revisionChanged);

    store.capture(QStringLiteral("claude"), QUrl(QStringLiteral("image://favicon/https://claude.ai/favicon.ico")));

    QCOMPARE(provider->requestedIds, (QStringList{QStringLiteral("https://claude.ai/favicon.ico")}));
    QCOMPARE(changes.size(), 1);
    const QUrl url = store.urlFor(QStringLiteral("claude"));
    QVERIFY(url.isLocalFile());
    QVERIFY(url.query().contains(QStringLiteral("rev=1")));
    const QImage saved(url.toLocalFile());
    QVERIFY(!saved.isNull());
    QCOMPARE(saved.pixelColor(saved.width() / 2, saved.height() / 2), QColor(Qt::red));
}

void TestFaviconStore::unchangedIconDoesNotBumpTheRevision()
{
    QQmlEngine engine;
    engine.addImageProvider(QStringLiteral("favicon"), new FakeFaviconProvider);
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());
    store.capture(QStringLiteral("a"), QUrl(QStringLiteral("image://favicon/x")));
    QSignalSpy changes(&store, &FaviconStore::revisionChanged);

    store.capture(QStringLiteral("a"), QUrl(QStringLiteral("image://favicon/x")));
    QCOMPARE(changes.size(), 0);

    store.capture(QStringLiteral("a"), QUrl(QStringLiteral("image://favicon/other")));
    QCOMPARE(changes.size(), 1);
}

void TestFaviconStore::ignoresOtherSchemesAndHosts()
{
    QQmlEngine engine;
    auto *provider = new FakeFaviconProvider;
    engine.addImageProvider(QStringLiteral("favicon"), provider);
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());

    store.capture(QStringLiteral("a"), QUrl(QStringLiteral("https://example.org/favicon.ico")));
    store.capture(QStringLiteral("a"), QUrl(QStringLiteral("image://other/icon")));
    store.capture(QStringLiteral("a"), QUrl());

    QVERIFY(provider->requestedIds.isEmpty());
    QVERIFY(store.urlFor(QStringLiteral("a")).isEmpty());
}

void TestFaviconStore::unknownProviderHasNoUrl()
{
    QQmlEngine engine;
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());
    QVERIFY(store.urlFor(QStringLiteral("nobody")).isEmpty());
    store.capture(QStringLiteral("nobody"), QUrl(QStringLiteral("image://favicon/x")));
    QVERIFY(store.urlFor(QStringLiteral("nobody")).isEmpty());
}

void TestFaviconStore::removeDeletesTheFile()
{
    QQmlEngine engine;
    engine.addImageProvider(QStringLiteral("favicon"), new FakeFaviconProvider);
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());
    store.capture(QStringLiteral("a"), QUrl(QStringLiteral("image://favicon/x")));
    QVERIFY(!store.urlFor(QStringLiteral("a")).isEmpty());

    store.remove(QStringLiteral("a"));

    QVERIFY(store.urlFor(QStringLiteral("a")).isEmpty());
}

namespace {

QByteArray pngBytes(const QColor &color)
{
    QImage image(32, 32, QImage::Format_ARGB32);
    image.fill(color);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

}

void TestFaviconStore::fetchesFaviconIcoWhenMissing()
{
    const QByteArray icon = pngBytes(Qt::green);
    MockServer server([icon](QTcpSocket *socket) {
        socket->write("HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\nConnection: close\r\nContent-Length: " + QByteArray::number(icon.size()) + "\r\n\r\n" + icon);
        socket->disconnectFromHost();
    });
    QQmlEngine engine;
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());
    QSignalSpy changes(&store, &FaviconStore::revisionChanged);

    store.fetchIfMissing(QStringLiteral("site"), server.baseUrl());

    QVERIFY(changes.wait(SignalTimeoutMs));
    QVERIFY(server.lastRequest().startsWith("GET /favicon.ico"));
    const QImage saved(store.urlFor(QStringLiteral("site")).toLocalFile());
    QCOMPARE(saved.pixelColor(saved.width() / 2, saved.height() / 2), QColor(Qt::green));
}

void TestFaviconStore::doesNotFetchWhenAlreadyStored()
{
    int requests = 0;
    MockServer server([&requests](QTcpSocket *socket) {
        ++requests;
        socket->disconnectFromHost();
    });
    QQmlEngine engine;
    engine.addImageProvider(QStringLiteral("favicon"), new FakeFaviconProvider);
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());
    store.capture(QStringLiteral("site"), QUrl(QStringLiteral("image://favicon/x")));

    store.fetchIfMissing(QStringLiteral("site"), server.baseUrl());
    QTest::qWait(200);

    QCOMPARE(requests, 0);
}

void TestFaviconStore::ignoresNonWebAddresses()
{
    QQmlEngine engine;
    QTemporaryDir directory;
    QNetworkAccessManager network;
    FaviconStore store(engine, network, directory.path());

    store.fetchIfMissing(QStringLiteral("site"), QUrl(QStringLiteral("file:///tmp/x")));
    store.fetchIfMissing(QStringLiteral("site"), QUrl());
    QTest::qWait(100);

    QVERIFY(store.urlFor(QStringLiteral("site")).isEmpty());
}

QTEST_MAIN(TestFaviconStore)

#include "tst_faviconstore.moc"
