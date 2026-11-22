// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/AppPaths.h"

#include "core/AppIdentity.h"

#include <QDir>
#include <QStandardPaths>

namespace {

constexpr QLatin1StringView ProfilesFolder{"profiles"};
constexpr QLatin1StringView OverridesFolder{"overrides"};
constexpr QLatin1StringView ChatDatabaseFileName{"chats.db"};

QString applicationFolder(QStandardPaths::StandardLocation location)
{
    return QDir(QStandardPaths::writableLocation(location)).filePath(QString(AppIdentity::ApplicationName));
}

}

QString AppPaths::profilesRoot()
{
    return QDir(applicationFolder(QStandardPaths::GenericDataLocation)).filePath(QString(ProfilesFolder));
}

QString AppPaths::profileDirectory(const QString &providerId)
{
    return QDir(profilesRoot()).filePath(providerId);
}

QString AppPaths::chatDatabaseFile()
{
    return QDir(applicationFolder(QStandardPaths::GenericDataLocation)).filePath(QString(ChatDatabaseFileName));
}

QString AppPaths::overridesDirectory()
{
    return QDir(applicationFolder(QStandardPaths::GenericConfigLocation)).filePath(QString(OverridesFolder));
}

QString AppPaths::downloadsDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
}
