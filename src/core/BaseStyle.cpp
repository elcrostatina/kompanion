// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/BaseStyle.h"

QString BaseStyle::css()
{
    return QStringLiteral("html { scrollbar-width: thin; scrollbar-color: rgba(128, 128, 128, 0.45) transparent; }");
}
