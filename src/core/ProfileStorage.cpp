// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ProfileStorage.h"

#include "core/EmptyFile.h"

#include <QDir>
#include <QFile>

namespace {

constexpr QLatin1StringView MarkerSuffix{".wipe"};

}

ProfileStorage::ProfileStorage(QString profilesDirectory)
    : m_profilesDirectory(std::move(profilesDirectory))
{
}

void ProfileStorage::scheduleWipe(const QString &providerId) const
{
    QDir().mkpath(m_profilesDirectory);
    EmptyFile::create(markerPath(providerId));
}

bool ProfileStorage::isWipeScheduled(const QString &providerId) const
{
    return QFile::exists(markerPath(providerId));
}

void ProfileStorage::applyScheduledWipes() const
{
    const QStringList markers = QDir(m_profilesDirectory).entryList({QStringLiteral("*") + QString(MarkerSuffix)}, QDir::Files);
    for (const QString &marker : markers) {
        const QString providerId = marker.chopped(MarkerSuffix.size());
        QDir(profilePath(providerId)).removeRecursively();
        QFile::remove(markerPath(providerId));
    }
}

QString ProfileStorage::markerPath(const QString &providerId) const
{
    return QDir(m_profilesDirectory).filePath(providerId + QString(MarkerSuffix));
}

QString ProfileStorage::profilePath(const QString &providerId) const
{
    return QDir(m_profilesDirectory).filePath(providerId);
}
