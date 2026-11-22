// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "app/PanelDocking.h"

class LayerShellSetup : public PanelDocking
{
public:
    void attach(QWindow *window, const PanelPlacement &placement) override;
    void place(QWindow *window, const PanelPlacement &placement) override;
};
