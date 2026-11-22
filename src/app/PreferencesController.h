// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

class AppSettings;
class Autostart;

class PreferencesController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")

    Q_PROPERTY(bool autostartEnabled READ autostartEnabled WRITE setAutostartEnabled NOTIFY autostartEnabledChanged FINAL)
    Q_PROPERTY(bool sidebarExpanded READ sidebarExpanded WRITE setSidebarExpanded NOTIFY sidebarExpandedChanged FINAL)
    Q_PROPERTY(bool followPlasmaColorScheme READ followPlasmaColorScheme WRITE setFollowPlasmaColorScheme NOTIFY followPlasmaColorSchemeChanged FINAL)
    Q_PROPERTY(QString defaultSystemPrompt READ defaultSystemPrompt WRITE setDefaultSystemPrompt NOTIFY defaultSystemPromptChanged FINAL)
    Q_PROPERTY(bool restartRequired READ restartRequired NOTIFY followPlasmaColorSchemeChanged FINAL)

public:
    PreferencesController(AppSettings &settings, Autostart &autostart, QObject *parent = nullptr);

    bool autostartEnabled() const;
    void setAutostartEnabled(bool enabled);
    bool sidebarExpanded() const;
    void setSidebarExpanded(bool expanded);
    bool followPlasmaColorScheme() const;
    void setFollowPlasmaColorScheme(bool follow);
    QString defaultSystemPrompt() const;
    void setDefaultSystemPrompt(const QString &prompt);
    bool restartRequired() const;

    Q_INVOKABLE void openShortcutSettings();

Q_SIGNALS:
    void autostartEnabledChanged();
    void sidebarExpandedChanged();
    void followPlasmaColorSchemeChanged();
    void defaultSystemPromptChanged();

private:
    AppSettings &m_settings;
    Autostart &m_autostart;
    bool m_colorSchemeAtStartup;
};
