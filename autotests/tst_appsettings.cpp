// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/AppSettings.h"
#include "core/PanelLimits.h"

#include <QTemporaryDir>
#include <QTest>

namespace {

KSharedConfigPtr openConfig(const QString &path)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    config->reparseConfiguration();
    return config;
}

}

class TestAppSettings : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_directory;

    QString configPath(const char *testName) const
    {
        return m_directory.filePath(QString::fromLatin1(testName));
    }

private Q_SLOTS:
    void defaultsToLeftSideAndDefaultWidth();
    void floatingDefaultsToOffAndPersists();
    void sidebarExpandedDefaultsToCollapsedAndPersists();
    void panelSideSurvivesReload();
    void panelWidthIsClampedAndPersisted();
    void unknownSideFallsBackToLeft();
    void lastProviderAndPromptArePersisted();
    void followColorSchemeDefaultsToTrue();
};

void TestAppSettings::defaultsToLeftSideAndDefaultWidth()
{
    const AppSettings settings(openConfig(configPath("defaults")));
    QCOMPARE(settings.panelSide(), PanelSide::Left);
    QCOMPARE(settings.panelWidth(), PanelLimits::DefaultWidth);
}

void TestAppSettings::floatingDefaultsToOffAndPersists()
{
    const QString path = configPath("floating");
    AppSettings settings(openConfig(path));
    QVERIFY(!settings.panelFloating());
    settings.setPanelFloating(true);
    QVERIFY(AppSettings(openConfig(path)).panelFloating());
}

void TestAppSettings::sidebarExpandedDefaultsToCollapsedAndPersists()
{
    const QString path = configPath("sidebar");
    AppSettings settings(openConfig(path));
    QVERIFY(!settings.sidebarExpanded());
    settings.setSidebarExpanded(true);
    QVERIFY(AppSettings(openConfig(path)).sidebarExpanded());
}

void TestAppSettings::panelSideSurvivesReload()
{
    const QString path = configPath("side");
    AppSettings(openConfig(path)).setPanelSide(PanelSide::Right);
    QCOMPARE(AppSettings(openConfig(path)).panelSide(), PanelSide::Right);
}

void TestAppSettings::panelWidthIsClampedAndPersisted()
{
    const QString path = configPath("width");
    AppSettings settings(openConfig(path));
    settings.setPanelWidth(PanelLimits::MaximumWidth * 3);
    QCOMPARE(AppSettings(openConfig(path)).panelWidth(), PanelLimits::MaximumWidth);
    settings.setPanelWidth(10);
    QCOMPARE(AppSettings(openConfig(path)).panelWidth(), PanelLimits::MinimumWidth);
    settings.setPanelWidth(500);
    QCOMPARE(AppSettings(openConfig(path)).panelWidth(), 500);
}

void TestAppSettings::unknownSideFallsBackToLeft()
{
    QCOMPARE(panelSideFromName(QStringLiteral("top")), PanelSide::Left);
    QCOMPARE(panelSideFromName(panelSideName(PanelSide::Right)), PanelSide::Right);
}

void TestAppSettings::lastProviderAndPromptArePersisted()
{
    const QString path = configPath("misc");
    AppSettings settings(openConfig(path));
    settings.setLastProviderId(QStringLiteral("ollama"));
    settings.setDefaultSystemPrompt(QStringLiteral("Be concise"));
    const AppSettings reloaded(openConfig(path));
    QCOMPARE(reloaded.lastProviderId(), QStringLiteral("ollama"));
    QCOMPARE(reloaded.defaultSystemPrompt(), QStringLiteral("Be concise"));
}

void TestAppSettings::followColorSchemeDefaultsToTrue()
{
    const QString path = configPath("scheme");
    AppSettings settings(openConfig(path));
    QVERIFY(settings.followPlasmaColorScheme());
    settings.setFollowPlasmaColorScheme(false);
    QVERIFY(!AppSettings(openConfig(path)).followPlasmaColorScheme());
}

QTEST_GUILESS_MAIN(TestAppSettings)

#include "tst_appsettings.moc"
