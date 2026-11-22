// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KConfigGroup>
#include "core/PanelSide.h"

#include <KSharedConfig>

class AppSettings
{
public:
    static bool readFollowPlasmaColorScheme();

    explicit AppSettings(KSharedConfigPtr config);

    int panelWidth() const;
    void setPanelWidth(int width);

    bool sidebarExpanded() const;
    void setSidebarExpanded(bool expanded);

    bool panelFloating() const;
    void setPanelFloating(bool floating);

    PanelSide panelSide() const;
    void setPanelSide(PanelSide side);

    QString lastProviderId() const;
    void setLastProviderId(const QString &providerId);

    QString defaultSystemPrompt() const;
    void setDefaultSystemPrompt(const QString &prompt);

    bool followPlasmaColorScheme() const;
    void setFollowPlasmaColorScheme(bool follow);

    bool firstRunCompleted() const;
    void markFirstRunCompleted();

private:
    KConfigGroup panelGroup() const;
    KConfigGroup generalGroup() const;

    KSharedConfigPtr m_config;
};
