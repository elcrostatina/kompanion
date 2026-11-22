// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/PanelSide.h"

#include <memory>

class QWindow;

struct PanelPlacement {
    int width;
    PanelSide side;
    bool floating;
};

class PanelDocking
{
public:
    virtual ~PanelDocking() = default;

    virtual void attach(QWindow *window, const PanelPlacement &placement) = 0;
    virtual void place(QWindow *window, const PanelPlacement &placement) = 0;
};

std::unique_ptr<PanelDocking> createPanelDocking();
