// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QUrl>

class NavigationPolicy
{
public:
    explicit NavigationPolicy(const QUrl &providerUrl);

    bool staysInsidePanel(const QUrl &target) const;

private:
    static QString siteOf(const QString &host);
    static bool isIdentityProviderHost(const QString &host);
    static bool isWebScheme(const QUrl &url);
    static bool isInternalScheme(const QUrl &url);

    QString m_providerSite;
};
