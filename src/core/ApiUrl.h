// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QUrl>

namespace ApiUrl {

QUrl endpoint(const QUrl &baseUrl, const QString &relativePath);

}
