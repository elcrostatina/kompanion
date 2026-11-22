// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/FileNaming.h"

#include <QFileInfo>

QString FileNaming::uniqueFileName(const QDir &directory, const QString &preferredName)
{
    const QFileInfo preferred(preferredName);
    const QString baseName = preferred.completeBaseName();
    const QString suffix = preferred.suffix().isEmpty() ? QString() : QLatin1Char('.') + preferred.suffix();
    QString candidate = preferredName;
    for (int counter = 1; directory.exists(candidate); ++counter) {
        candidate = QStringLiteral("%1 (%2)%3").arg(baseName, QString::number(counter), suffix);
    }
    return candidate;
}
