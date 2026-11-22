// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>

class OverrideFiles;

class OverrideReloader : public QObject
{
    Q_OBJECT

public:
    OverrideReloader(const OverrideFiles &files, const QString &providerId, QObject *parent = nullptr);

Q_SIGNALS:
    void filesChanged();

private:
    void watchExistingFiles();

    QStringList m_filePaths;
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
};
