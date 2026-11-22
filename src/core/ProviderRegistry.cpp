// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ProviderRegistry.h"

#include "core/ProviderPresets.h"

#include <KConfigGroup>

#include <QRegularExpression>

#include <algorithm>

namespace {

constexpr QLatin1StringView RegistryGroupName{"Providers"};
constexpr QLatin1StringView ProviderGroupPrefix{"Provider-"};
constexpr QLatin1StringView OrderKey{"Order"};
constexpr QLatin1StringView LegacySeededKey{"Seeded"};
constexpr QLatin1StringView SeededPresetsKey{"SeededPresets"};
constexpr QLatin1StringView NameKey{"Name"};
constexpr QLatin1StringView KindKey{"Kind"};
constexpr QLatin1StringView EnabledKey{"Enabled"};
constexpr QLatin1StringView IconKey{"Icon"};
constexpr QLatin1StringView UrlKey{"Url"};
constexpr QLatin1StringView UserAgentKey{"UserAgent"};
constexpr QLatin1StringView OverridesEnabledKey{"OverridesEnabled"};
constexpr QLatin1StringView ZoomFactorKey{"ZoomFactor"};
constexpr QLatin1StringView DefaultModelKey{"DefaultModel"};
constexpr QLatin1StringView FallbackIdBase{"provider"};

QString providerGroupName(const QString &id)
{
    return QString(ProviderGroupPrefix) + id;
}

QString slugify(const QString &name)
{
    static const QRegularExpression unsafeCharacters(QStringLiteral("[^a-z0-9]+"));
    QString slug = name.toLower();
    slug.replace(unsafeCharacters, QStringLiteral("-"));
    slug.remove(QRegularExpression(QStringLiteral("^-+|-+$")));
    return slug.isEmpty() ? QString(FallbackIdBase) : slug;
}

QString uniqueId(const QString &name, const QList<Provider> &existing)
{
    const QString base = slugify(name);
    const auto isTaken = [&existing](const QString &candidate) {
        return std::any_of(existing.cbegin(), existing.cend(), [&candidate](const Provider &provider) {
            return provider.id == candidate;
        });
    };
    QString candidate = base;
    for (int suffix = 2; isTaken(candidate); ++suffix) {
        candidate = base + QLatin1Char('-') + QString::number(suffix);
    }
    return candidate;
}

}

ProviderRegistry::ProviderRegistry(KSharedConfigPtr config, QObject *parent)
    : QObject(parent)
    , m_config(std::move(config))
{
    load();
}

void ProviderRegistry::seedMissingPresets()
{
    KConfigGroup registryGroup(m_config, RegistryGroupName);
    QStringList seededIds = registryGroup.readEntry(SeededPresetsKey, QStringList());
    if (seededIds.isEmpty() && registryGroup.readEntry(LegacySeededKey, false)) {
        seededIds = ProviderPresets::idsFromFirstRelease();
    }
    bool addedProvider = false;
    for (const Provider &preset : ProviderPresets::defaults()) {
        if (seededIds.contains(preset.id)) {
            continue;
        }
        seededIds.append(preset.id);
        if (indexOf(preset.id) < 0) {
            m_providers.insert(insertionIndexFor(preset.kind), preset);
            writeProvider(preset);
            addedProvider = true;
        }
    }
    registryGroup.writeEntry(SeededPresetsKey, seededIds);
    if (addedProvider) {
        writeOrder();
    }
    flush();
    if (addedProvider) {
        Q_EMIT providersChanged();
    }
}

QList<Provider> ProviderRegistry::providers() const
{
    return m_providers;
}

QList<Provider> ProviderRegistry::enabledProviders() const
{
    QList<Provider> enabled;
    std::copy_if(m_providers.cbegin(), m_providers.cend(), std::back_inserter(enabled), [](const Provider &provider) {
        return provider.enabled;
    });
    return enabled;
}

std::optional<Provider> ProviderRegistry::find(const QString &id) const
{
    const qsizetype index = indexOf(id);
    if (index < 0) {
        return std::nullopt;
    }
    return m_providers.at(index);
}

QString ProviderRegistry::add(Provider provider)
{
    provider.id = uniqueId(provider.name, m_providers);
    m_providers.insert(insertionIndexFor(provider.kind), provider);
    writeProvider(provider);
    writeOrder();
    flush();
    Q_EMIT providersChanged();
    return provider.id;
}

void ProviderRegistry::update(const Provider &provider)
{
    const qsizetype index = indexOf(provider.id);
    if (index < 0 || m_providers.at(index) == provider) {
        return;
    }
    m_providers[index] = provider;
    writeProvider(provider);
    flush();
    Q_EMIT providerUpdated(provider.id);
}

void ProviderRegistry::remove(const QString &id)
{
    const qsizetype index = indexOf(id);
    if (index < 0) {
        return;
    }
    m_providers.removeAt(index);
    m_config->deleteGroup(providerGroupName(id));
    writeOrder();
    flush();
    Q_EMIT providersChanged();
}

void ProviderRegistry::move(int from, int to)
{
    const bool inRange = from >= 0 && to >= 0 && from < m_providers.size() && to < m_providers.size();
    if (!inRange || from == to) {
        return;
    }
    m_providers.move(from, to);
    writeOrder();
    flush();
    Q_EMIT providersChanged();
}

void ProviderRegistry::load()
{
    m_providers.clear();
    for (const QString &id : storedIdsInOrder()) {
        m_providers.append(readProvider(id));
    }
}

QStringList ProviderRegistry::storedIdsInOrder() const
{
    QStringList ordered = KConfigGroup(m_config, RegistryGroupName).readEntry(OrderKey, QStringList());
    const QStringList groups = m_config->groupList();
    ordered.erase(std::remove_if(ordered.begin(), ordered.end(), [&groups](const QString &id) {
        return !groups.contains(providerGroupName(id));
    }), ordered.end());
    for (const QString &group : groups) {
        const bool isProviderGroup = group.startsWith(ProviderGroupPrefix);
        const QString id = group.mid(ProviderGroupPrefix.size());
        if (isProviderGroup && !ordered.contains(id)) {
            ordered.append(id);
        }
    }
    return ordered;
}

Provider ProviderRegistry::readProvider(const QString &id) const
{
    const Provider defaults;
    const KConfigGroup group(m_config, providerGroupName(id));
    Provider provider;
    provider.id = id;
    provider.name = group.readEntry(NameKey, id);
    provider.kind = providerKindFromName(group.readEntry(KindKey, providerKindName(ProviderKind::Web)));
    provider.enabled = group.readEntry(EnabledKey, defaults.enabled);
    provider.iconName = group.readEntry(IconKey, QString());
    provider.url = QUrl(group.readEntry(UrlKey, QString()));
    provider.userAgent = group.readEntry(UserAgentKey, QString());
    provider.overridesEnabled = group.readEntry(OverridesEnabledKey, defaults.overridesEnabled);
    provider.zoomFactor = group.readEntry(ZoomFactorKey, defaults.zoomFactor);
    provider.defaultModel = group.readEntry(DefaultModelKey, QString());
    return provider;
}

void ProviderRegistry::writeProvider(const Provider &provider)
{
    KConfigGroup group(m_config, providerGroupName(provider.id));
    group.writeEntry(NameKey, provider.name);
    group.writeEntry(KindKey, providerKindName(provider.kind));
    group.writeEntry(EnabledKey, provider.enabled);
    group.writeEntry(IconKey, provider.iconName);
    group.writeEntry(UrlKey, provider.url.toString());
    group.writeEntry(UserAgentKey, provider.userAgent);
    group.writeEntry(OverridesEnabledKey, provider.overridesEnabled);
    group.writeEntry(ZoomFactorKey, provider.zoomFactor);
    group.writeEntry(DefaultModelKey, provider.defaultModel);
}

void ProviderRegistry::writeOrder()
{
    QStringList ids;
    for (const Provider &provider : std::as_const(m_providers)) {
        ids.append(provider.id);
    }
    KConfigGroup(m_config, RegistryGroupName).writeEntry(OrderKey, ids);
}

void ProviderRegistry::flush()
{
    m_config->sync();
}

qsizetype ProviderRegistry::insertionIndexFor(ProviderKind kind) const
{
    for (qsizetype index = m_providers.size() - 1; index >= 0; --index) {
        if (m_providers.at(index).kind == kind) {
            return index + 1;
        }
    }
    return m_providers.size();
}

qsizetype ProviderRegistry::indexOf(const QString &id) const
{
    const auto match = std::find_if(m_providers.cbegin(), m_providers.cend(), [&id](const Provider &provider) {
        return provider.id == id;
    });
    return match == m_providers.cend() ? -1 : std::distance(m_providers.cbegin(), match);
}
