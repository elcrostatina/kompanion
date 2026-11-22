// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/PreferencesController.h"

#include "core/AppSettings.h"
#include "core/Autostart.h"

#include <QProcess>

namespace {

constexpr QLatin1StringView SystemSettingsExecutable{"systemsettings"};
constexpr QLatin1StringView ShortcutsModule{"kcm_keys"};

}

PreferencesController::PreferencesController(AppSettings &settings, Autostart &autostart, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_autostart(autostart)
    , m_colorSchemeAtStartup(settings.followPlasmaColorScheme())
{
}

bool PreferencesController::autostartEnabled() const
{
    return m_autostart.isEnabled();
}

void PreferencesController::setAutostartEnabled(bool enabled)
{
    m_autostart.setEnabled(enabled);
    Q_EMIT autostartEnabledChanged();
}

bool PreferencesController::sidebarExpanded() const
{
    return m_settings.sidebarExpanded();
}

void PreferencesController::setSidebarExpanded(bool expanded)
{
    m_settings.setSidebarExpanded(expanded);
    Q_EMIT sidebarExpandedChanged();
}

bool PreferencesController::followPlasmaColorScheme() const
{
    return m_settings.followPlasmaColorScheme();
}

void PreferencesController::setFollowPlasmaColorScheme(bool follow)
{
    m_settings.setFollowPlasmaColorScheme(follow);
    Q_EMIT followPlasmaColorSchemeChanged();
}

QString PreferencesController::defaultSystemPrompt() const
{
    return m_settings.defaultSystemPrompt();
}

void PreferencesController::setDefaultSystemPrompt(const QString &prompt)
{
    m_settings.setDefaultSystemPrompt(prompt);
    Q_EMIT defaultSystemPromptChanged();
}

bool PreferencesController::restartRequired() const
{
    return m_colorSchemeAtStartup != m_settings.followPlasmaColorScheme();
}

void PreferencesController::openShortcutSettings()
{
    QProcess::startDetached(QString(SystemSettingsExecutable), {QString(ShortcutsModule)});
}
