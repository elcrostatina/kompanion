// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/AppSettings.h"

#include "core/PanelLimits.h"

namespace {

constexpr QLatin1StringView PanelGroupName{"Panel"};
constexpr QLatin1StringView GeneralGroupName{"General"};
constexpr QLatin1StringView WidthKey{"Width"};
constexpr QLatin1StringView SideKey{"Side"};
constexpr QLatin1StringView FloatingKey{"Floating"};
constexpr QLatin1StringView SidebarExpandedKey{"SidebarExpanded"};
constexpr QLatin1StringView FirstRunCompletedKey{"FirstRunCompleted"};
constexpr QLatin1StringView LastProviderKey{"LastProvider"};
constexpr QLatin1StringView DefaultSystemPromptKey{"DefaultSystemPrompt"};
constexpr QLatin1StringView FollowColorSchemeKey{"FollowPlasmaColorScheme"};
constexpr QLatin1StringView ConfigFileName{"kompanionrc"};

}

bool AppSettings::readFollowPlasmaColorScheme()
{
    const KConfig config{QString(ConfigFileName), KConfig::SimpleConfig};
    return KConfigGroup(&config, GeneralGroupName).readEntry(FollowColorSchemeKey, true);
}

AppSettings::AppSettings(KSharedConfigPtr config)
    : m_config(std::move(config))
{
}

int AppSettings::panelWidth() const
{
    return PanelLimits::clampWidth(panelGroup().readEntry(WidthKey, PanelLimits::DefaultWidth));
}

void AppSettings::setPanelWidth(int width)
{
    KConfigGroup group = panelGroup();
    group.writeEntry(WidthKey, PanelLimits::clampWidth(width));
    m_config->sync();
}

bool AppSettings::sidebarExpanded() const
{
    return panelGroup().readEntry(SidebarExpandedKey, false);
}

void AppSettings::setSidebarExpanded(bool expanded)
{
    KConfigGroup group = panelGroup();
    group.writeEntry(SidebarExpandedKey, expanded);
    m_config->sync();
}

bool AppSettings::panelFloating() const
{
    return panelGroup().readEntry(FloatingKey, false);
}

void AppSettings::setPanelFloating(bool floating)
{
    KConfigGroup group = panelGroup();
    group.writeEntry(FloatingKey, floating);
    m_config->sync();
}

PanelSide AppSettings::panelSide() const
{
    return panelSideFromName(panelGroup().readEntry(SideKey, panelSideName(PanelSide::Left)));
}

void AppSettings::setPanelSide(PanelSide side)
{
    KConfigGroup group = panelGroup();
    group.writeEntry(SideKey, panelSideName(side));
    m_config->sync();
}

QString AppSettings::lastProviderId() const
{
    return generalGroup().readEntry(LastProviderKey, QString());
}

void AppSettings::setLastProviderId(const QString &providerId)
{
    KConfigGroup group = generalGroup();
    group.writeEntry(LastProviderKey, providerId);
    m_config->sync();
}

QString AppSettings::defaultSystemPrompt() const
{
    return generalGroup().readEntry(DefaultSystemPromptKey, QString());
}

void AppSettings::setDefaultSystemPrompt(const QString &prompt)
{
    KConfigGroup group = generalGroup();
    group.writeEntry(DefaultSystemPromptKey, prompt);
    m_config->sync();
}

bool AppSettings::followPlasmaColorScheme() const
{
    return generalGroup().readEntry(FollowColorSchemeKey, true);
}

void AppSettings::setFollowPlasmaColorScheme(bool follow)
{
    KConfigGroup group = generalGroup();
    group.writeEntry(FollowColorSchemeKey, follow);
    m_config->sync();
}

bool AppSettings::firstRunCompleted() const
{
    return generalGroup().readEntry(FirstRunCompletedKey, false);
}

void AppSettings::markFirstRunCompleted()
{
    KConfigGroup group = generalGroup();
    group.writeEntry(FirstRunCompletedKey, true);
    m_config->sync();
}

KConfigGroup AppSettings::panelGroup() const
{
    return KConfigGroup(m_config, PanelGroupName);
}

KConfigGroup AppSettings::generalGroup() const
{
    return KConfigGroup(m_config, GeneralGroupName);
}
