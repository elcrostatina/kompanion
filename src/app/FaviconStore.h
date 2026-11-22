// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSet>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QQmlEngine;

class FaviconStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged FINAL)

public:
    FaviconStore(QQmlEngine &engine, QNetworkAccessManager &network, QString directory, QObject *parent = nullptr);

    int revision() const;

    Q_INVOKABLE void capture(const QString &providerId, const QUrl &iconUrl);
    Q_INVOKABLE QUrl urlFor(const QString &providerId) const;
    Q_INVOKABLE void remove(const QString &providerId);
    void fetchIfMissing(const QString &providerId, const QUrl &siteUrl);

Q_SIGNALS:
    void revisionChanged();

private:
    QString pathFor(const QString &providerId) const;
    QImage requestImage(const QUrl &iconUrl) const;
    bool writeIfChanged(const QString &path, const QImage &image) const;
    void storeImage(const QString &providerId, const QImage &image);
    void handleFetched(const QString &providerId, QNetworkReply *reply);

    QQmlEngine &m_engine;
    QNetworkAccessManager &m_network;
    QSet<QString> m_pending;
    QString m_directory;
    int m_revision = 0;
};
