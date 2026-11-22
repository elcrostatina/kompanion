// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>

namespace PanelLimits {

inline constexpr int DefaultWidth = 440;
inline constexpr int MinimumWidth = 320;
inline constexpr int MaximumWidth = 1200;
inline constexpr int FloatingGap = 8;

constexpr int clampWidth(int width)
{
    return std::clamp(width, MinimumWidth, MaximumWidth);
}

}
