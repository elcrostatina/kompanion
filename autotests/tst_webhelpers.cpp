// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/AppIdentity.h"
#include "core/Autostart.h"
#include "core/FileNaming.h"
#include "core/NavigationPolicy.h"
#include "core/ProfileStorage.h"
#include "core/UserAgent.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

namespace {

bool staysInside(const QString &providerUrl, const QString &target)
{
    return NavigationPolicy(QUrl(providerUrl)).staysInsidePanel(QUrl(target));
}

void touch(const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
}

}

class TestWebHelpers : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void userAgentLosesEngineToken();
    void userAgentWithoutTokenIsUntouched();
    void sameSiteAndSubdomainsStayInside();
    void otherSitesLeaveThePanel();
    void identityProvidersStayInside();
    void mailAndTelLeaveThePanel();
    void internalSchemesStayInside();
    void countrySecondLevelDomainsAreHandled();
    void uniqueFileNameKeepsFreeName();
    void uniqueFileNameAddsCounterBeforeSuffix();
    void scheduledWipeRemovesProfileOnApply();
    void unscheduledProfilesAreKept();
    void autostartWritesAndRemovesDesktopFile();
    void dbusServiceNameEqualsTheApplicationId();
};

void TestWebHelpers::userAgentLosesEngineToken()
{
    const QString engine = QStringLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) QtWebEngine/6.11.1 Chrome/142.0.0.0 Safari/537.36");
    QCOMPARE(UserAgent::withoutEngineToken(engine),
             QStringLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36"));
}

void TestWebHelpers::userAgentWithoutTokenIsUntouched()
{
    const QString plain = QStringLiteral("Mozilla/5.0 Chrome/142.0.0.0");
    QCOMPARE(UserAgent::withoutEngineToken(plain), plain);
}

void TestWebHelpers::sameSiteAndSubdomainsStayInside()
{
    QVERIFY(staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("https://claude.ai/chat/1")));
    QVERIFY(staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("https://api.claude.ai/x")));
    QVERIFY(staysInside(QStringLiteral("https://gemini.google.com"), QStringLiteral("https://www.google.com/search")));
}

void TestWebHelpers::otherSitesLeaveThePanel()
{
    QVERIFY(!staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("https://example.org/page")));
    QVERIFY(!staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("https://notclaude.ai/")));
    QVERIFY(!staysInside(QStringLiteral("https://chatgpt.com"), QStringLiteral("https://en.wikipedia.org/wiki/Qt")));
}

void TestWebHelpers::identityProvidersStayInside()
{
    QVERIFY(staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("https://accounts.google.com/o/oauth2/auth")));
    QVERIFY(staysInside(QStringLiteral("https://copilot.microsoft.com"), QStringLiteral("https://login.live.com/oauth20")));
    QVERIFY(staysInside(QStringLiteral("https://chatgpt.com"), QStringLiteral("https://auth.openai.com/authorize")));
}

void TestWebHelpers::mailAndTelLeaveThePanel()
{
    QVERIFY(!staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("mailto:someone@example.org")));
    QVERIFY(!staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("tel:+123456")));
}

void TestWebHelpers::internalSchemesStayInside()
{
    QVERIFY(staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("about:blank")));
    QVERIFY(staysInside(QStringLiteral("https://claude.ai"), QStringLiteral("blob:https://claude.ai/abc")));
}

void TestWebHelpers::countrySecondLevelDomainsAreHandled()
{
    QVERIFY(staysInside(QStringLiteral("https://chat.example.co.uk"), QStringLiteral("https://www.example.co.uk/")));
    QVERIFY(!staysInside(QStringLiteral("https://chat.example.co.uk"), QStringLiteral("https://other.co.uk/")));
}

void TestWebHelpers::uniqueFileNameKeepsFreeName()
{
    QTemporaryDir directory;
    QCOMPARE(FileNaming::uniqueFileName(QDir(directory.path()), QStringLiteral("report.pdf")), QStringLiteral("report.pdf"));
}

void TestWebHelpers::uniqueFileNameAddsCounterBeforeSuffix()
{
    QTemporaryDir directory;
    touch(directory.filePath(QStringLiteral("report.pdf")));
    QCOMPARE(FileNaming::uniqueFileName(QDir(directory.path()), QStringLiteral("report.pdf")), QStringLiteral("report (1).pdf"));
    touch(directory.filePath(QStringLiteral("report (1).pdf")));
    QCOMPARE(FileNaming::uniqueFileName(QDir(directory.path()), QStringLiteral("report.pdf")), QStringLiteral("report (2).pdf"));
}

void TestWebHelpers::scheduledWipeRemovesProfileOnApply()
{
    QTemporaryDir directory;
    const ProfileStorage storage(directory.filePath(QStringLiteral("profiles")));
    touch(directory.filePath(QStringLiteral("profiles/claude/Cookies")));
    storage.scheduleWipe(QStringLiteral("claude"));
    QVERIFY(storage.isWipeScheduled(QStringLiteral("claude")));
    QVERIFY(QDir(directory.filePath(QStringLiteral("profiles/claude"))).exists());

    storage.applyScheduledWipes();

    QVERIFY(!QDir(directory.filePath(QStringLiteral("profiles/claude"))).exists());
    QVERIFY(!storage.isWipeScheduled(QStringLiteral("claude")));
}

void TestWebHelpers::unscheduledProfilesAreKept()
{
    QTemporaryDir directory;
    const ProfileStorage storage(directory.filePath(QStringLiteral("profiles")));
    touch(directory.filePath(QStringLiteral("profiles/gemini/Cookies")));
    touch(directory.filePath(QStringLiteral("profiles/claude/Cookies")));
    storage.scheduleWipe(QStringLiteral("claude"));
    storage.applyScheduledWipes();
    QVERIFY(QFile::exists(directory.filePath(QStringLiteral("profiles/gemini/Cookies"))));
}

void TestWebHelpers::autostartWritesAndRemovesDesktopFile()
{
    QTemporaryDir directory;
    const Autostart autostart(directory.filePath(QStringLiteral("autostart")), QStringLiteral("/opt/my apps/kompanion"));
    QVERIFY(!autostart.isEnabled());
    QVERIFY(autostart.setEnabled(true));
    QVERIFY(autostart.isEnabled());

    QFile file(directory.filePath(QStringLiteral("autostart/io.github.elcrostatina.Kompanion.desktop")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QString content = QString::fromUtf8(file.readAll());
    QVERIFY(content.contains(QStringLiteral("Exec=\"/opt/my apps/kompanion\"")));

    QVERIFY(autostart.setEnabled(false));
    QVERIFY(!autostart.isEnabled());
    QVERIFY(autostart.setEnabled(false));
}

void TestWebHelpers::dbusServiceNameEqualsTheApplicationId()
{
    QStringList domainLabels = QString(AppIdentity::OrganizationDomain).split(QLatin1Char('.'));
    std::reverse(domainLabels.begin(), domainLabels.end());
    const QString serviceName = domainLabels.join(QLatin1Char('.')) + QLatin1Char('.') + QString(AppIdentity::DisplayName);
    QCOMPARE(serviceName, QString(AppIdentity::ApplicationId));
}

QTEST_GUILESS_MAIN(TestWebHelpers)

#include "tst_webhelpers.moc"
