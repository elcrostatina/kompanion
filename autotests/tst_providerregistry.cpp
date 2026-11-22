// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ProviderRegistry.h"

#include <KConfigGroup>

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace {

KSharedConfigPtr openConfig(const QString &path)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    config->reparseConfiguration();
    return config;
}

QStringList idsOf(const QList<Provider> &providers)
{
    QStringList ids;
    for (const Provider &provider : providers) {
        ids.append(provider.id);
    }
    return ids;
}

Provider customProvider(const QString &name)
{
    Provider provider;
    provider.name = name;
    provider.url = QUrl(QStringLiteral("https://example.org/chat"));
    return provider;
}

}

class TestProviderRegistry : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_directory;

    QString configPath(const char *testName) const
    {
        return m_directory.filePath(QString::fromLatin1(testName));
    }

private Q_SLOTS:
    void startsEmptyWithoutSeeding();
    void seedsPresetsOnce();
    void addsNewPresetsToExistingInstallOnce();
    void presetsContainAllWebAndLocalProviders();
    void addGeneratesUniqueIds();
    void updatesSurviveReload();
    void removeDeletesProvider();
    void moveChangesOrderAndPersists();
    void enabledProvidersExcludeDisabled();
    void updateSignalsOnlyOnRealChange();
};

void TestProviderRegistry::startsEmptyWithoutSeeding()
{
    ProviderRegistry registry(openConfig(configPath("empty")));
    QVERIFY(registry.providers().isEmpty());
}

void TestProviderRegistry::seedsPresetsOnce()
{
    const QString path = configPath("seed");
    ProviderRegistry registry(openConfig(path));
    registry.seedMissingPresets();
    const int count = registry.providers().size();
    QVERIFY(count > 0);
    registry.remove(QStringLiteral("claude"));
    registry.seedMissingPresets();
    QCOMPARE(registry.providers().size(), count - 1);

    ProviderRegistry reloaded(openConfig(path));
    reloaded.seedMissingPresets();
    QCOMPARE(reloaded.providers().size(), count - 1);
}

void TestProviderRegistry::addsNewPresetsToExistingInstallOnce()
{
    const QString path = configPath("migrate");
    {
        KSharedConfigPtr config = openConfig(path);
        KConfigGroup registry(config, QStringLiteral("Providers"));
        registry.writeEntry("Seeded", true);
        registry.writeEntry("Order", QStringList{QStringLiteral("claude"), QStringLiteral("ollama")});
        KConfigGroup ollama(config, QStringLiteral("Provider-ollama"));
        ollama.writeEntry("Name", QStringLiteral("Ollama"));
        ollama.writeEntry("Kind", QStringLiteral("local"));
        KConfigGroup claude(config, QStringLiteral("Provider-claude"));
        claude.writeEntry("Name", QStringLiteral("Claude"));
        claude.writeEntry("Url", QStringLiteral("https://claude.ai"));
        config->sync();
    }
    ProviderRegistry registry(openConfig(path));
    registry.seedMissingPresets();
    QVERIFY(registry.find(QStringLiteral("lumo")).has_value());
    QCOMPARE(registry.find(QStringLiteral("lumo"))->url, QUrl(QStringLiteral("https://lumo.proton.me")));
    QVERIFY(!registry.find(QStringLiteral("gemini")).has_value());
    QCOMPARE(idsOf(registry.providers()).mid(0, 3), (QStringList{QStringLiteral("claude"), QStringLiteral("lumo"), QStringLiteral("ollama")}));

    registry.remove(QStringLiteral("lumo"));
    ProviderRegistry reloaded(openConfig(path));
    reloaded.seedMissingPresets();
    QVERIFY(!reloaded.find(QStringLiteral("lumo")).has_value());
}

void TestProviderRegistry::presetsContainAllWebAndLocalProviders()
{
    ProviderRegistry registry(openConfig(configPath("presets")));
    registry.seedMissingPresets();
    const QStringList ids = idsOf(registry.providers());
    for (const QString &expected : {QStringLiteral("claude"), QStringLiteral("chatgpt"), QStringLiteral("copilot"), QStringLiteral("gemini"), QStringLiteral("lumo"), QStringLiteral("lmstudio"), QStringLiteral("ollama")}) {
        QVERIFY2(ids.contains(expected), qPrintable(expected));
    }
    QCOMPARE(registry.find(QStringLiteral("claude"))->url, QUrl(QStringLiteral("https://claude.ai")));
    QCOMPARE(registry.find(QStringLiteral("lmstudio"))->url, QUrl(QStringLiteral("http://localhost:1234/v1")));
    QCOMPARE(registry.find(QStringLiteral("ollama"))->url, QUrl(QStringLiteral("http://localhost:11434/v1")));
    QCOMPARE(registry.find(QStringLiteral("ollama"))->kind, ProviderKind::Local);
}

void TestProviderRegistry::addGeneratesUniqueIds()
{
    ProviderRegistry registry(openConfig(configPath("add")));
    const QString first = registry.add(customProvider(QStringLiteral("My Bot!")));
    const QString second = registry.add(customProvider(QStringLiteral("My Bot!")));
    QCOMPARE(first, QStringLiteral("my-bot"));
    QCOMPARE(second, QStringLiteral("my-bot-2"));
}

void TestProviderRegistry::updatesSurviveReload()
{
    const QString path = configPath("update");
    {
        ProviderRegistry registry(openConfig(path));
        const QString id = registry.add(customProvider(QStringLiteral("Bot")));
        Provider provider = *registry.find(id);
        provider.name = QStringLiteral("Renamed");
        provider.userAgent = QStringLiteral("Custom/1.0");
        provider.zoomFactor = 1.5;
        provider.overridesEnabled = false;
        provider.enabled = false;
        registry.update(provider);
    }
    ProviderRegistry reloaded(openConfig(path));
    const std::optional<Provider> provider = reloaded.find(QStringLiteral("bot"));
    QVERIFY(provider.has_value());
    QCOMPARE(provider->name, QStringLiteral("Renamed"));
    QCOMPARE(provider->userAgent, QStringLiteral("Custom/1.0"));
    QCOMPARE(provider->zoomFactor, 1.5);
    QVERIFY(!provider->overridesEnabled);
    QVERIFY(!provider->enabled);
    QCOMPARE(provider->url, QUrl(QStringLiteral("https://example.org/chat")));
}

void TestProviderRegistry::removeDeletesProvider()
{
    const QString path = configPath("remove");
    ProviderRegistry registry(openConfig(path));
    const QString id = registry.add(customProvider(QStringLiteral("Temp")));
    registry.remove(id);
    QVERIFY(!registry.find(id).has_value());
    ProviderRegistry reloaded(openConfig(path));
    QVERIFY(reloaded.providers().isEmpty());
}

void TestProviderRegistry::moveChangesOrderAndPersists()
{
    const QString path = configPath("move");
    ProviderRegistry registry(openConfig(path));
    registry.add(customProvider(QStringLiteral("One")));
    registry.add(customProvider(QStringLiteral("Two")));
    registry.add(customProvider(QStringLiteral("Three")));
    registry.move(2, 0);
    QCOMPARE(idsOf(registry.providers()), (QStringList{QStringLiteral("three"), QStringLiteral("one"), QStringLiteral("two")}));
    registry.move(9, 0);
    QCOMPARE(registry.providers().size(), 3);
    ProviderRegistry reloaded(openConfig(path));
    QCOMPARE(idsOf(reloaded.providers()), (QStringList{QStringLiteral("three"), QStringLiteral("one"), QStringLiteral("two")}));
}

void TestProviderRegistry::enabledProvidersExcludeDisabled()
{
    ProviderRegistry registry(openConfig(configPath("enabled")));
    registry.seedMissingPresets();
    const int enabledBefore = registry.enabledProviders().size();
    Provider claude = *registry.find(QStringLiteral("claude"));
    claude.enabled = false;
    registry.update(claude);
    QCOMPARE(registry.enabledProviders().size(), enabledBefore - 1);
    QVERIFY(!idsOf(registry.enabledProviders()).contains(QStringLiteral("claude")));
}

void TestProviderRegistry::updateSignalsOnlyOnRealChange()
{
    ProviderRegistry registry(openConfig(configPath("signals")));
    const QString id = registry.add(customProvider(QStringLiteral("Bot")));
    QSignalSpy updated(&registry, &ProviderRegistry::providerUpdated);
    registry.update(*registry.find(id));
    QCOMPARE(updated.size(), 0);
    Provider changed = *registry.find(id);
    changed.name = QStringLiteral("Other");
    registry.update(changed);
    QCOMPARE(updated.size(), 1);
}

QTEST_GUILESS_MAIN(TestProviderRegistry)

#include "tst_providerregistry.moc"
