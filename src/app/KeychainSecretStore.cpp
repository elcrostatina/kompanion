// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/KeychainSecretStore.h"

#include "core/AppIdentity.h"

#include <qt6keychain/keychain.h>

namespace {

template<typename Job>
Job *makeJob(const QString &key, QObject *parent)
{
    auto *job = new Job(QString(AppIdentity::ApplicationId), parent);
    job->setAutoDelete(false);
    job->setKey(key);
    return job;
}

bool succeededOrMissing(const QKeychain::Job *job)
{
    return job->error() == QKeychain::NoError || job->error() == QKeychain::EntryNotFound;
}

}

KeychainSecretStore::KeychainSecretStore(QObject *parent)
    : QObject(parent)
{
}

void KeychainSecretStore::read(const QString &key, ReadCallback done)
{
    auto *job = makeJob<QKeychain::ReadPasswordJob>(key, this);
    connect(job, &QKeychain::Job::finished, this, [job, done] {
        const bool found = job->error() == QKeychain::NoError;
        done(found ? std::optional<QString>(job->textData()) : std::nullopt);
        job->deleteLater();
    });
    job->start();
}

void KeychainSecretStore::write(const QString &key, const QString &secret, CompletionCallback done)
{
    auto *job = makeJob<QKeychain::WritePasswordJob>(key, this);
    job->setTextData(secret);
    connect(job, &QKeychain::Job::finished, this, [job, done] {
        done(job->error() == QKeychain::NoError);
        job->deleteLater();
    });
    job->start();
}

void KeychainSecretStore::remove(const QString &key, CompletionCallback done)
{
    auto *job = makeJob<QKeychain::DeletePasswordJob>(key, this);
    connect(job, &QKeychain::Job::finished, this, [job, done] {
        done(succeededOrMissing(job));
        job->deleteLater();
    });
    job->start();
}
