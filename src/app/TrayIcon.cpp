// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/TrayIcon.h"

#include "app/PanelController.h"
#include "app/ProviderSelection.h"
#include "core/AppIdentity.h"
#include "core/ProviderRegistry.h"

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QIcon>
#include <QMenu>

namespace {

constexpr QLatin1StringView FallbackIconName{"applications-internet"};

QIcon applicationIcon()
{
    return QIcon::fromTheme(QString(AppIdentity::ApplicationId), QIcon::fromTheme(QString(FallbackIconName)));
}

}

TrayIcon::TrayIcon(PanelController &panel, ProviderRegistry &registry, ProviderSelection &selection, QObject *parent)
    : QObject(parent)
    , m_panel(panel)
    , m_registry(registry)
    , m_selection(selection)
    , m_item(new KStatusNotifierItem(QString(AppIdentity::ApplicationId), this))
    , m_menu(new QMenu)
    , m_toggleAction(m_menu->addAction(QString(), &m_panel, &PanelController::toggle))
    , m_providerGroup(new QActionGroup(this))
{
    m_menu->addSeparator();
    m_providersEnd = m_menu->addSeparator();
    m_menu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), i18n("Quit"), qApp, &QApplication::quit);

    m_item->setCategory(KStatusNotifierItem::ApplicationStatus);
    m_item->setStatus(KStatusNotifierItem::Active);
    m_item->setTitle(QString(AppIdentity::DisplayName));
    m_item->setToolTip(applicationIcon(), QString(AppIdentity::DisplayName), i18n("AI chat sidebar"));
    m_item->setIconByPixmap(applicationIcon());
    m_item->setStandardActionsEnabled(false);
    m_item->setContextMenu(m_menu);

    connect(m_item, &KStatusNotifierItem::activateRequested, &m_panel, &PanelController::toggle);
    connect(&m_panel, &PanelController::shownChanged, this, &TrayIcon::updateToggleText);
    connect(&m_registry, &ProviderRegistry::providersChanged, this, &TrayIcon::rebuildProviderActions);
    connect(&m_registry, &ProviderRegistry::providerUpdated, this, &TrayIcon::rebuildProviderActions);
    connect(&m_selection, &ProviderSelection::currentChanged, this, &TrayIcon::updateCheckedProvider);
    updateToggleText();
    rebuildProviderActions();
}

void TrayIcon::rebuildProviderActions()
{
    for (QAction *action : m_providerGroup->actions()) {
        m_menu->removeAction(action);
        m_providerGroup->removeAction(action);
        action->deleteLater();
    }
    const QList<Provider> providers = m_registry.enabledProviders();
    for (const Provider &provider : providers) {
        QAction *action = new QAction(provider.name, m_providerGroup);
        action->setCheckable(true);
        action->setData(provider.id);
        connect(action, &QAction::triggered, this, [this, id = provider.id] { chooseProvider(id); });
        m_menu->insertAction(m_providersEnd, action);
    }
    updateCheckedProvider();
}

void TrayIcon::updateToggleText()
{
    m_toggleAction->setText(m_panel.isShown() ? i18n("Hide Kompanion") : i18n("Show Kompanion"));
}

void TrayIcon::updateCheckedProvider()
{
    for (QAction *action : m_providerGroup->actions()) {
        action->setChecked(action->data().toString() == m_selection.currentId());
    }
}

void TrayIcon::chooseProvider(const QString &providerId)
{
    m_selection.selectId(providerId);
    m_panel.show();
}
