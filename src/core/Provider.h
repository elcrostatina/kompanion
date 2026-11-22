// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QUrl>

enum class ProviderKind {
    Web,
    Local,
};

namespace ProviderKindNames {

inline constexpr QLatin1StringView Web{"web"};
inline constexpr QLatin1StringView Local{"local"};

}

inline QString providerKindName(ProviderKind kind)
{
    return QString(kind == ProviderKind::Web ? ProviderKindNames::Web : ProviderKindNames::Local);
}

inline ProviderKind providerKindFromName(const QString &name)
{
    return name == ProviderKindNames::Local ? ProviderKind::Local : ProviderKind::Web;
}

struct Provider {
    QString id;
    QString name;
    ProviderKind kind = ProviderKind::Web;
    bool enabled = true;
    QString iconName;
    QUrl url;
    QString userAgent;
    bool overridesEnabled = true;
    double zoomFactor = 1.0;
    QString defaultModel;

    bool operator==(const Provider &other) const = default;
};
