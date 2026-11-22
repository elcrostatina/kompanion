// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/InMemorySecretStore.h"

void InMemorySecretStore::read(const QString &key, ReadCallback done)
{
    const auto match = m_secrets.constFind(key);
    done(match == m_secrets.cend() ? std::nullopt : std::optional<QString>(*match));
}

void InMemorySecretStore::write(const QString &key, const QString &secret, CompletionCallback done)
{
    m_secrets.insert(key, secret);
    done(true);
}

void InMemorySecretStore::remove(const QString &key, CompletionCallback done)
{
    m_secrets.remove(key);
    done(true);
}
