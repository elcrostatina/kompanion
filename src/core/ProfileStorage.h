// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class ProfileStorage
{
public:
    explicit ProfileStorage(QString profilesDirectory);

    void scheduleWipe(const QString &providerId) const;
    bool isWipeScheduled(const QString &providerId) const;
    void applyScheduledWipes() const;

private:
    QString markerPath(const QString &providerId) const;
    QString profilePath(const QString &providerId) const;

    QString m_profilesDirectory;
};
