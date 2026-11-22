// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/UserAgent.h"

#include <QRegularExpression>

QString UserAgent::withoutEngineToken(const QString &engineUserAgent)
{
    static const QRegularExpression engineToken(QStringLiteral(" QtWebEngine/[0-9.]+"));
    QString userAgent = engineUserAgent;
    userAgent.remove(engineToken);
    return userAgent;
}
