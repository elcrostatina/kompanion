// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Autostart.h"

#include "core/AppIdentity.h"

#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>

namespace {

constexpr QLatin1StringView AutostartFolder{"autostart"};
constexpr QLatin1StringView DesktopSuffix{".desktop"};

}

Autostart::Autostart(QString autostartDirectory, QString executablePath)
    : m_directory(std::move(autostartDirectory))
    , m_executablePath(std::move(executablePath))
{
}

QString Autostart::defaultDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).filePath(QString(AutostartFolder));
}

bool Autostart::isEnabled() const
{
    return QFile::exists(desktopFilePath());
}

bool Autostart::setEnabled(bool enabled) const
{
    if (!enabled) {
        return !QFile::exists(desktopFilePath()) || QFile::remove(desktopFilePath());
    }
    if (!QDir().mkpath(m_directory)) {
        return false;
    }
    QSaveFile file(desktopFilePath());
    return file.open(QIODevice::WriteOnly) && file.write(desktopEntry()) >= 0 && file.commit();
}

QString Autostart::desktopFilePath() const
{
    return QDir(m_directory).filePath(QString(AppIdentity::ApplicationId) + QString(DesktopSuffix));
}

QByteArray Autostart::desktopEntry() const
{
    const QString entry = QStringLiteral("[Desktop Entry]\nType=Application\nName=%1\nExec=\"%2\"\nIcon=%3\nTerminal=false\nX-GNOME-Autostart-enabled=true\n")
                              .arg(AppIdentity::DisplayName, m_executablePath, AppIdentity::ApplicationId);
    return entry.toUtf8();
}
