// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/NavigationPolicy.h"

#include <QStringList>

#include <algorithm>

namespace {

constexpr int ShortestRegistrableLabelCount = 2;
constexpr int CountryCodeLength = 2;
constexpr int GenericSecondLevelMaxLength = 3;

const QStringList &identityProviderHosts()
{
    static const QStringList hosts{
        QStringLiteral("accounts.google.com"),
        QStringLiteral("login.microsoftonline.com"),
        QStringLiteral("login.microsoft.com"),
        QStringLiteral("login.live.com"),
        QStringLiteral("appleid.apple.com"),
        QStringLiteral("auth.openai.com"),
        QStringLiteral("github.com"),
    };
    return hosts;
}

bool isSameOrSubdomain(const QString &host, const QString &parent)
{
    return host == parent || host.endsWith(QLatin1Char('.') + parent);
}

}

NavigationPolicy::NavigationPolicy(const QUrl &providerUrl)
    : m_providerSite(siteOf(providerUrl.host().toLower()))
{
}

bool NavigationPolicy::staysInsidePanel(const QUrl &target) const
{
    if (isInternalScheme(target)) {
        return true;
    }
    if (!isWebScheme(target)) {
        return false;
    }
    const QString host = target.host().toLower();
    return siteOf(host) == m_providerSite || isIdentityProviderHost(host);
}

QString NavigationPolicy::siteOf(const QString &host)
{
    const QStringList labels = host.split(QLatin1Char('.'), Qt::SkipEmptyParts);
    if (labels.size() <= ShortestRegistrableLabelCount) {
        return host;
    }
    const QString &topLevel = labels.at(labels.size() - 1);
    const QString &secondLevel = labels.at(labels.size() - 2);
    const bool isCountrySecondLevel = topLevel.size() == CountryCodeLength && secondLevel.size() <= GenericSecondLevelMaxLength;
    const int siteLabelCount = isCountrySecondLevel ? ShortestRegistrableLabelCount + 1 : ShortestRegistrableLabelCount;
    return labels.mid(labels.size() - siteLabelCount).join(QLatin1Char('.'));
}

bool NavigationPolicy::isIdentityProviderHost(const QString &host)
{
    const QStringList &hosts = identityProviderHosts();
    return std::any_of(hosts.cbegin(), hosts.cend(), [&host](const QString &identityHost) {
        return isSameOrSubdomain(host, identityHost);
    });
}

bool NavigationPolicy::isWebScheme(const QUrl &url)
{
    return url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https");
}

bool NavigationPolicy::isInternalScheme(const QUrl &url)
{
    static const QStringList internalSchemes{QStringLiteral("about"), QStringLiteral("blob"), QStringLiteral("data"), QStringLiteral("qrc")};
    return internalSchemes.contains(url.scheme());
}
