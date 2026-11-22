// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include <functional>
#include <optional>

class SecretStore
{
public:
    using ReadCallback = std::function<void(const std::optional<QString> &secret)>;
    using CompletionCallback = std::function<void(bool succeeded)>;

    virtual ~SecretStore() = default;

    virtual void read(const QString &key, ReadCallback done) = 0;
    virtual void write(const QString &key, const QString &secret, CompletionCallback done) = 0;
    virtual void remove(const QString &key, CompletionCallback done) = 0;
};

namespace SecretKeys {

QString apiKeyFor(const QString &providerId);

}
