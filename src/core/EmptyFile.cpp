// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/EmptyFile.h"

#include <QFile>

bool EmptyFile::create(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly);
}
