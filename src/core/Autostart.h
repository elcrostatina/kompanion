// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class Autostart
{
public:
    Autostart(QString autostartDirectory, QString executablePath);

    static QString defaultDirectory();

    bool isEnabled() const;
    bool setEnabled(bool enabled) const;

private:
    QString desktopFilePath() const;
    QByteArray desktopEntry() const;

    QString m_directory;
    QString m_executablePath;
};
