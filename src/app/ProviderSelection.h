// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class AppSettings;
class ProviderRegistry;

class ProviderSelection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentId READ currentId NOTIFY currentChanged FINAL)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentChanged FINAL)

public:
    ProviderSelection(ProviderRegistry &registry, AppSettings &settings, QObject *parent = nullptr);

    QString currentId() const;
    int currentIndex() const;

    Q_INVOKABLE void selectId(const QString &providerId);
    Q_INVOKABLE void selectIndex(int index);

Q_SIGNALS:
    void currentChanged();

private:
    QStringList enabledIds() const;
    void keepSelectionValid();

    ProviderRegistry &m_registry;
    AppSettings &m_settings;
    QString m_currentId;
};
