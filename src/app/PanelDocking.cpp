// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/PanelDocking.h"

#include "app/FramelessDocking.h"
#include "app/LayerShellSetup.h"

#include <QGuiApplication>

namespace {

bool isWaylandSession()
{
    return QGuiApplication::platformName() == QLatin1StringView("wayland");
}

}

std::unique_ptr<PanelDocking> createPanelDocking()
{
    if (isWaylandSession()) {
        return std::make_unique<LayerShellSetup>();
    }
    return std::make_unique<FramelessDocking>();
}
