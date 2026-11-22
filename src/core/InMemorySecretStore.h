// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/SecretStore.h"

#include <QHash>

class InMemorySecretStore : public SecretStore
{
public:
    void read(const QString &key, ReadCallback done) override;
    void write(const QString &key, const QString &secret, CompletionCallback done) override;
    void remove(const QString &key, CompletionCallback done) override;

private:
    QHash<QString, QString> m_secrets;
};
