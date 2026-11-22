// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class PanelController;
class ProviderRegistry;
class ProviderSelection;
class KStatusNotifierItem;
class QAction;
class QActionGroup;
class QMenu;

class TrayIcon : public QObject
{
    Q_OBJECT

public:
    TrayIcon(PanelController &panel, ProviderRegistry &registry, ProviderSelection &selection, QObject *parent = nullptr);

private:
    void rebuildProviderActions();
    void updateToggleText();
    void updateCheckedProvider();
    void chooseProvider(const QString &providerId);

    PanelController &m_panel;
    ProviderRegistry &m_registry;
    ProviderSelection &m_selection;
    KStatusNotifierItem *m_item;
    QMenu *m_menu;
    QAction *m_toggleAction;
    QActionGroup *m_providerGroup;
    QAction *m_providersEnd;
};
