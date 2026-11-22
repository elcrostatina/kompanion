// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ShortcutManager.h"

#include "core/AppIdentity.h"
#include "core/AppSettings.h"

#include <KGlobalAccel>
#include <KLocalizedString>

#include <QAction>

namespace {

constexpr QLatin1StringView ToggleActionName{"toggle-panel"};
constexpr QLatin1StringView ComponentNameProperty{"componentName"};
constexpr QLatin1StringView ComponentDisplayNameProperty{"componentDisplayName"};

}

ShortcutManager::ShortcutManager(AppSettings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

void ShortcutManager::registerToggleShortcut()
{
    announceConflictOnFirstRun();
    m_toggleAction = createToggleAction();
    const QList<QKeySequence> shortcuts{defaultSequence()};
    KGlobalAccel::self()->setDefaultShortcut(m_toggleAction, shortcuts, KGlobalAccel::Autoloading);
    KGlobalAccel::self()->setShortcut(m_toggleAction, shortcuts, KGlobalAccel::Autoloading);
}

QKeySequence ShortcutManager::defaultSequence()
{
    return QKeySequence(Qt::META | Qt::Key_C);
}

QAction *ShortcutManager::createToggleAction()
{
    auto *action = new QAction(i18n("Show or hide Kompanion"), this);
    action->setObjectName(ToggleActionName);
    action->setProperty(ComponentNameProperty.constData(), QString(AppIdentity::ApplicationId));
    action->setProperty(ComponentDisplayNameProperty.constData(), QString(AppIdentity::DisplayName));
    connect(action, &QAction::triggered, this, &ShortcutManager::toggleRequested);
    return action;
}

void ShortcutManager::announceConflictOnFirstRun()
{
    if (m_settings.firstRunCompleted()) {
        return;
    }
    if (!KGlobalAccel::isGlobalShortcutAvailable(defaultSequence())) {
        Q_EMIT defaultShortcutUnavailable(defaultSequence().toString(QKeySequence::NativeText));
    }
    m_settings.markFirstRunCompleted();
}
