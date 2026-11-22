// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

namespace ChatRole {

inline constexpr QLatin1StringView System{"system"};
inline constexpr QLatin1StringView User{"user"};
inline constexpr QLatin1StringView Assistant{"assistant"};

}

struct ChatMessage {
    QString role;
    QString content;

    bool operator==(const ChatMessage &other) const = default;
};
