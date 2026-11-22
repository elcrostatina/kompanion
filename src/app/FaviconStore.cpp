// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/FaviconStore.h"

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QUrlQuery>

namespace {

constexpr QLatin1StringView FaviconProviderName{"favicon"};
constexpr QLatin1StringView ImageScheme{"image"};
constexpr QLatin1StringView PngExtension{".png"};
constexpr QLatin1StringView RevisionQueryKey{"rev"};
constexpr int StoredIconSize = 64;
constexpr int FetchTimeoutMs = 15000;
constexpr QLatin1StringView FaviconPath{"/favicon.ico"};

QByteArray encodedPng(const QImage &image)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

QByteArray readAll(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

}

FaviconStore::FaviconStore(QQmlEngine &engine, QNetworkAccessManager &network, QString directory, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_network(network)
    , m_directory(std::move(directory))
{
}

int FaviconStore::revision() const
{
    return m_revision;
}

void FaviconStore::capture(const QString &providerId, const QUrl &iconUrl)
{
    storeImage(providerId, requestImage(iconUrl));
}

void FaviconStore::fetchIfMissing(const QString &providerId, const QUrl &siteUrl)
{
    const bool isWebAddress = siteUrl.scheme() == QLatin1String("http") || siteUrl.scheme() == QLatin1String("https");
    if (!isWebAddress || siteUrl.host().isEmpty() || QFile::exists(pathFor(providerId)) || m_pending.contains(providerId)) {
        return;
    }
    QUrl iconUrl;
    iconUrl.setScheme(siteUrl.scheme());
    iconUrl.setHost(siteUrl.host());
    iconUrl.setPort(siteUrl.port());
    iconUrl.setPath(QString(FaviconPath));
    QNetworkRequest request(iconUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(FetchTimeoutMs);
    m_pending.insert(providerId);
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, providerId, reply] {
        handleFetched(providerId, reply);
    });
}

QUrl FaviconStore::urlFor(const QString &providerId) const
{
    const QString path = pathFor(providerId);
    if (!QFile::exists(path)) {
        return QUrl();
    }
    QUrl url = QUrl::fromLocalFile(path);
    QUrlQuery query;
    query.addQueryItem(QString(RevisionQueryKey), QString::number(m_revision));
    url.setQuery(query);
    return url;
}

void FaviconStore::remove(const QString &providerId)
{
    if (QFile::remove(pathFor(providerId))) {
        ++m_revision;
        Q_EMIT revisionChanged();
    }
}

void FaviconStore::handleFetched(const QString &providerId, QNetworkReply *reply)
{
    reply->deleteLater();
    m_pending.remove(providerId);
    if (reply->error() == QNetworkReply::NoError && !QFile::exists(pathFor(providerId))) {
        storeImage(providerId, QImage::fromData(reply->readAll()));
    }
}

void FaviconStore::storeImage(const QString &providerId, const QImage &image)
{
    if (image.isNull()) {
        return;
    }
    const QImage stored = image.scaled(StoredIconSize, StoredIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (writeIfChanged(pathFor(providerId), stored)) {
        ++m_revision;
        Q_EMIT revisionChanged();
    }
}

QString FaviconStore::pathFor(const QString &providerId) const
{
    return QDir(m_directory).filePath(providerId + QString(PngExtension));
}

QImage FaviconStore::requestImage(const QUrl &iconUrl) const
{
    if (iconUrl.scheme() != ImageScheme || iconUrl.host() != FaviconProviderName) {
        return QImage();
    }
    auto *provider = dynamic_cast<QQuickImageProvider *>(m_engine.imageProvider(QString(FaviconProviderName)));
    if (!provider || provider->imageType() != QQmlImageProviderBase::Image) {
        return QImage();
    }
    const QString imageId = iconUrl.toString().mid(QStringLiteral("image://favicon/").size());
    QSize size;
    return provider->requestImage(imageId, &size, QSize(StoredIconSize, StoredIconSize));
}

bool FaviconStore::writeIfChanged(const QString &path, const QImage &image) const
{
    const QByteArray bytes = encodedPng(image);
    if (bytes == readAll(path)) {
        return false;
    }
    QDir().mkpath(m_directory);
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}
