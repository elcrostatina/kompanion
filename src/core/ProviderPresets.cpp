// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/ProviderPresets.h"

namespace {

constexpr QLatin1StringView LocalProviderIcon{"computer"};

Provider webPreset(const QString &id, const QString &name, const QString &address)
{
    Provider provider;
    provider.id = id;
    provider.name = name;
    provider.kind = ProviderKind::Web;
    provider.url = QUrl(address);
    return provider;
}

Provider localPreset(const QString &id, const QString &name, const QString &address)
{
    Provider provider;
    provider.id = id;
    provider.name = name;
    provider.kind = ProviderKind::Local;
    provider.enabled = false;
    provider.iconName = QString(LocalProviderIcon);
    provider.url = QUrl(address);
    return provider;
}

}

QStringList ProviderPresets::idsFromFirstRelease()
{
    return {QStringLiteral("claude"), QStringLiteral("chatgpt"), QStringLiteral("copilot"), QStringLiteral("gemini"), QStringLiteral("lmstudio"), QStringLiteral("ollama")};
}

QList<Provider> ProviderPresets::defaults()
{
    return {
        webPreset(QStringLiteral("claude"), QStringLiteral("Claude"), QStringLiteral("https://claude.ai")),
        webPreset(QStringLiteral("chatgpt"), QStringLiteral("ChatGPT"), QStringLiteral("https://chatgpt.com")),
        webPreset(QStringLiteral("copilot"), QStringLiteral("Microsoft Copilot"), QStringLiteral("https://copilot.microsoft.com")),
        webPreset(QStringLiteral("gemini"), QStringLiteral("Gemini"), QStringLiteral("https://gemini.google.com")),
        webPreset(QStringLiteral("lumo"), QStringLiteral("Lumo"), QStringLiteral("https://lumo.proton.me")),
        localPreset(QStringLiteral("lmstudio"), QStringLiteral("LM Studio"), QStringLiteral("http://localhost:1234/v1")),
        localPreset(QStringLiteral("ollama"), QStringLiteral("Ollama"), QStringLiteral("http://localhost:11434/v1")),
    };
}
