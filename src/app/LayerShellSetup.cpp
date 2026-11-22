// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/LayerShellSetup.h"

#include "core/AppIdentity.h"
#include "core/PanelLimits.h"

#include <LayerShellQt/Window>

#include <QMargins>
#include <QSize>

namespace {

constexpr int NoExclusiveZone = 0;
constexpr int FillAvailableHeight = 0;

LayerShellQt::Window::Anchors edgeAnchors(PanelSide side)
{
    const LayerShellQt::Window::Anchor sideAnchor = side == PanelSide::Right ? LayerShellQt::Window::AnchorRight : LayerShellQt::Window::AnchorLeft;
    return LayerShellQt::Window::Anchors(sideAnchor | LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom);
}

}

void LayerShellSetup::attach(QWindow *window, const PanelPlacement &placement)
{
    LayerShellQt::Window *layerWindow = LayerShellQt::Window::get(window);
    layerWindow->setScope(QString(AppIdentity::ApplicationName));
    layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWindow->setExclusiveZone(NoExclusiveZone);
    layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layerWindow->setWantsToBeOnActiveScreen(true);
    layerWindow->setActivateOnShow(true);
    place(window, placement);
}

void LayerShellSetup::place(QWindow *window, const PanelPlacement &placement)
{
    LayerShellQt::Window *layerWindow = LayerShellQt::Window::get(window);
    const int gap = placement.floating ? PanelLimits::FloatingGap : 0;
    layerWindow->setAnchors(edgeAnchors(placement.side));
    layerWindow->setMargins(QMargins(gap, gap, gap, gap));
    layerWindow->setDesiredSize(QSize(placement.width, FillAvailableHeight));
}
