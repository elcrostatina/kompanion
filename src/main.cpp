// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ChatControllerRegistry.h"
#include "app/FaviconStore.h"
#include "app/KeychainSecretStore.h"
#include "app/PanelController.h"
#include "app/PanelDocking.h"
#include "app/PreferencesController.h"
#include "app/ProviderEditor.h"
#include "app/ProviderModel.h"
#include "app/ProviderSelection.h"
#include "app/ShortcutManager.h"
#include "app/TrayIcon.h"
#include "app/WebProfileRegistry.h"
#include "core/AppIdentity.h"
#include "core/AppPaths.h"
#include "core/AppSettings.h"
#include "core/Autostart.h"
#include "core/ChatStore.h"
#include "core/OverrideFiles.h"
#include "core/ProfileStorage.h"
#include "core/ProviderRegistry.h"

#include <KDBusService>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QNetworkAccessManager>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtWebEngineQuick>

namespace {

constexpr QLatin1StringView DesktopQuickStyle{"org.kde.desktop"};
constexpr QLatin1StringView QuickStyleEnvironmentVariable{"QT_QUICK_CONTROLS_STYLE"};
constexpr QLatin1StringView TranslationDomain{"kompanion"};
constexpr QLatin1StringView MainQmlType{"Main"};
constexpr QLatin1StringView ChromiumFlagsEnvironmentVariable{"QTWEBENGINE_CHROMIUM_FLAGS"};
constexpr QLatin1StringView LightColorSchemeFlag{"--blink-settings=preferredColorScheme=0"};
constexpr QLatin1StringView SettingsOptionName{"settings"};
constexpr int StartupFailure = -1;

void appendChromiumFlag(QByteArray &flags, QLatin1StringView flag)
{
    flags.append(' ');
    flags.append(flag.data());
}

void applyChromiumFlags()
{
    QByteArray flags = qgetenv(ChromiumFlagsEnvironmentVariable.constData());
    if (!AppSettings::readFollowPlasmaColorScheme()) {
        appendChromiumFlag(flags, LightColorSchemeFlag);
    }
    qputenv(ChromiumFlagsEnvironmentVariable.constData(), flags);
}

void prepareGraphicsEnvironment()
{
    applyChromiumFlags();
    QtWebEngineQuick::initialize();
    QQuickWindow::setDefaultAlphaBuffer(true);
}

void configureApplicationIdentity(QApplication &application)
{
    application.setApplicationName(QString(AppIdentity::DisplayName));
    application.setOrganizationDomain(QString(AppIdentity::OrganizationDomain));
    application.setApplicationDisplayName(QString(AppIdentity::DisplayName));
    application.setDesktopFileName(QString(AppIdentity::ApplicationId));
    application.setQuitOnLastWindowClosed(false);
    KLocalizedString::setApplicationDomain(QByteArray(TranslationDomain.data()));
}

void applyDesktopQuickStyleUnlessOverridden()
{
    if (qEnvironmentVariableIsEmpty(QuickStyleEnvironmentVariable.constData())) {
        QQuickStyle::setStyle(QString(DesktopQuickStyle));
    }
}

void reportQmlWarnings(const QList<QQmlError> &warnings)
{
    for (const QQmlError &warning : warnings) {
        qWarning().noquote() << warning.toString();
    }
}

QVariantMap qmlInitialProperties(PanelController &panel,
                                 ProviderModel &enabledProviders,
                                 ProviderSelection &selection,
                                 WebProfileRegistry &webProfiles,
                                 ChatControllerRegistry &chatControllers,
                                 ProviderModel &allProviders,
                                 PreferencesController &preferences,
                                 ProviderEditor &providerEditor,
                                 FaviconStore &favicons)
{
    return {
        {QStringLiteral("panel"), QVariant::fromValue(&panel)},
        {QStringLiteral("enabledProviders"), QVariant::fromValue(&enabledProviders)},
        {QStringLiteral("providerSelection"), QVariant::fromValue(&selection)},
        {QStringLiteral("webProfiles"), QVariant::fromValue(&webProfiles)},
        {QStringLiteral("chatControllers"), QVariant::fromValue(&chatControllers)},
        {QStringLiteral("allProviders"), QVariant::fromValue(&allProviders)},
        {QStringLiteral("preferences"), QVariant::fromValue(&preferences)},
        {QStringLiteral("providerEditor"), QVariant::fromValue(&providerEditor)},
        {QStringLiteral("favicons"), QVariant::fromValue(&favicons)},
    };
}

QQuickWindow *loadPanelWindow(QQmlApplicationEngine &engine, const QVariantMap &initialProperties)
{
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &reportQmlWarnings);
    KLocalization::setupLocalizedContext(&engine);
    engine.setInitialProperties(initialProperties);
    engine.loadFromModule(AppIdentity::QmlModuleUri, MainQmlType);
    return engine.rootObjects().isEmpty() ? nullptr : qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
}

void fetchMissingFavicons(const ProviderRegistry &providers, FaviconStore &favicons)
{
    const QList<Provider> all = providers.providers();
    for (const Provider &provider : all) {
        if (provider.kind == ProviderKind::Web) {
            favicons.fetchIfMissing(provider.id, provider.url);
        }
    }
}

QString shortcutConflictMessage(const QString &shortcutText)
{
    return i18n("The shortcut %1 is already used by something else. Choose another one in System Settings > Shortcuts > Kompanion.", shortcutText);
}

bool asksForSettings(const QStringList &arguments)
{
    QCommandLineParser parser;
    const QCommandLineOption settingsOption(QString(SettingsOptionName), i18n("Open the settings page"));
    parser.addOption(settingsOption);
    parser.parse(arguments);
    return parser.isSet(settingsOption);
}

void handleActivation(PanelController &panel, const QStringList &arguments)
{
    if (asksForSettings(arguments)) {
        panel.showSettings();
    } else {
        panel.toggle();
    }
}

void wireShortcutsAndActivation(PanelController &panel, ShortcutManager &shortcuts, KDBusService &service)
{
    QObject::connect(&shortcuts, &ShortcutManager::toggleRequested, &panel, &PanelController::toggle);
    QObject::connect(&service, &KDBusService::activateRequested, &panel, [&panel](const QStringList &arguments) {
        handleActivation(panel, arguments);
    });
    QObject::connect(&shortcuts, &ShortcutManager::defaultShortcutUnavailable, &panel, [&panel](const QString &shortcutText) {
        panel.postNotice(shortcutConflictMessage(shortcutText));
        panel.show();
    });
}

}

int main(int argc, char *argv[])
{
    prepareGraphicsEnvironment();
    QApplication application(argc, argv);
    configureApplicationIdentity(application);
    applyDesktopQuickStyleUnlessOverridden();

    KDBusService service(KDBusService::Unique);

    const KSharedConfigPtr config = KSharedConfig::openConfig(QString(AppIdentity::ConfigFileName));
    AppSettings settings(config);
    ProviderRegistry providers(config);
    providers.seedMissingPresets();
    OverrideFiles overrideFiles(AppPaths::overridesDirectory());
    ProfileStorage profileStorage(AppPaths::profilesRoot());
    profileStorage.applyScheduledWipes();
    Autostart autostart(Autostart::defaultDirectory(), QCoreApplication::applicationFilePath());

    const std::unique_ptr<PanelDocking> docking = createPanelDocking();
    PanelController panel(settings, *docking);
    ShortcutManager shortcuts(settings);
    ProviderModel enabledProviders(providers, ProviderModel::Filter::EnabledOnly);
    ProviderSelection selection(providers, settings);
    WebProfileRegistry webProfiles(providers, overrideFiles);
    ChatStore chatStore(AppPaths::chatDatabaseFile());
    KeychainSecretStore secrets;
    QNetworkAccessManager network;
    ChatControllerRegistry chatControllers(chatStore, providers, settings, secrets, network);
    ProviderModel allProviders(providers, ProviderModel::Filter::AllProviders);
    PreferencesController preferences(settings, autostart);
    ProviderEditor providerEditor(providers, overrideFiles, profileStorage, webProfiles, secrets);
    TrayIcon trayIcon(panel, providers, selection);

    QQmlApplicationEngine engine;
    FaviconStore favicons(engine, network, AppPaths::faviconsDirectory());
    fetchMissingFavicons(providers, favicons);
    QObject::connect(&providers, &ProviderRegistry::providersChanged, &favicons, [&providers, &favicons] {
        fetchMissingFavicons(providers, favicons);
    });
    QObject::connect(&providerEditor, &ProviderEditor::providerRemoved, &favicons, &FaviconStore::remove);
    QQuickWindow *window = loadPanelWindow(engine, qmlInitialProperties(panel, enabledProviders, selection, webProfiles, chatControllers, allProviders, preferences, providerEditor, favicons));
    if (!window) {
        return StartupFailure;
    }

    panel.attachWindow(window);
    wireShortcutsAndActivation(panel, shortcuts, service);
    shortcuts.registerToggleShortcut();
    if (asksForSettings(QCoreApplication::arguments())) {
        panel.showSettings();
    }

    return application.exec();
}
