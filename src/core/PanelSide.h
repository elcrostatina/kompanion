// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLatin1StringView>
#include <QString>

enum class PanelSide {
    Left,
    Right,
};

namespace PanelSideNames {

inline constexpr QLatin1StringView Left{"left"};
inline constexpr QLatin1StringView Right{"right"};

}

inline QString panelSideName(PanelSide side)
{
    return QString(side == PanelSide::Right ? PanelSideNames::Right : PanelSideNames::Left);
}

inline PanelSide panelSideFromName(const QString &name)
{
    return name == PanelSideNames::Right ? PanelSide::Right : PanelSide::Left;
}
