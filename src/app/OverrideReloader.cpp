// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/OverrideReloader.h"

#include "core/OverrideFiles.h"

#include <QFile>

namespace {

constexpr int SaveSettleDelayMs = 150;

}

OverrideReloader::OverrideReloader(const OverrideFiles &files, const QString &providerId, QObject *parent)
    : QObject(parent)
    , m_filePaths({files.cssPath(providerId), files.javaScriptPath(providerId)})
{
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(SaveSettleDelayMs);
    m_watcher.addPath(files.directory());
    watchExistingFiles();

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, &m_debounce, qOverload<>(&QTimer::start));
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, &m_debounce, qOverload<>(&QTimer::start));
    connect(&m_debounce, &QTimer::timeout, this, [this] {
        watchExistingFiles();
        Q_EMIT filesChanged();
    });
}

void OverrideReloader::watchExistingFiles()
{
    const QStringList alreadyWatched = m_watcher.files();
    for (const QString &path : std::as_const(m_filePaths)) {
        if (QFile::exists(path) && !alreadyWatched.contains(path)) {
            m_watcher.addPath(path);
        }
    }
}
