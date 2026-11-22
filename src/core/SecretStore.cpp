// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/SecretStore.h"

QString SecretKeys::apiKeyFor(const QString &providerId)
{
    return QStringLiteral("provider-%1-api-key").arg(providerId);
}
