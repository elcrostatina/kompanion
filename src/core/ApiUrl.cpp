// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ApiUrl.h"

namespace {

constexpr QLatin1StringView DefaultApiPath{"/v1"};

QString normalizedBasePath(const QUrl &baseUrl)
{
    QString path = baseUrl.path();
    while (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    return path.isEmpty() ? QString(DefaultApiPath) : path;
}

}

QUrl ApiUrl::endpoint(const QUrl &baseUrl, const QString &relativePath)
{
    QUrl url = baseUrl;
    url.setPath(normalizedBasePath(baseUrl) + QLatin1Char('/') + relativePath);
    return url;
}
