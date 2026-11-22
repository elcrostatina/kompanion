// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QString>

namespace FileNaming {

QString uniqueFileName(const QDir &directory, const QString &preferredName);

}
