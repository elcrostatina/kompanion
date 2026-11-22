// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QKeySequence>
#include <QObject>

class AppSettings;
class QAction;

class ShortcutManager : public QObject
{
    Q_OBJECT

public:
    explicit ShortcutManager(AppSettings &settings, QObject *parent = nullptr);

    void registerToggleShortcut();

Q_SIGNALS:
    void toggleRequested();
    void defaultShortcutUnavailable(const QString &shortcutText);

private:
    static QKeySequence defaultSequence();
    QAction *createToggleAction();
    void announceConflictOnFirstRun();

    AppSettings &m_settings;
    QAction *m_toggleAction = nullptr;
};
