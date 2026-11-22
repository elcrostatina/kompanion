// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "app/OverrideReloader.h"
#include "core/NavigationPolicy.h"
#include "core/Provider.h"

#include <QObject>
#include <QQmlEngine>
#include <QQuickWebEngineProfile>
#include <QUrl>

class OverrideFiles;
class ProviderRegistry;
class QQuickWebEngineDownloadRequest;

class WebProviderProfile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by WebProfileRegistry")
    Q_PROPERTY(QQuickWebEngineProfile *profile READ profile CONSTANT FINAL)
    Q_PROPERTY(QString providerId READ providerId CONSTANT FINAL)
    Q_PROPERTY(QString providerName READ providerName NOTIFY providerChanged FINAL)
    Q_PROPERTY(QUrl startUrl READ startUrl NOTIFY providerChanged FINAL)
    Q_PROPERTY(double zoomFactor READ zoomFactor NOTIFY providerChanged FINAL)
    Q_PROPERTY(QString styleScript READ styleScript NOTIFY scriptsChanged FINAL)
    Q_PROPERTY(QString userScript READ userScript NOTIFY scriptsChanged FINAL)
    Q_PROPERTY(QString pageColorScript READ pageColorScript CONSTANT FINAL)

public:
    WebProviderProfile(const Provider &provider, OverrideFiles &files, ProviderRegistry &registry, QObject *parent = nullptr);

    void applyProvider(const Provider &provider);

    QQuickWebEngineProfile *profile() const;
    QString providerId() const;
    QString providerName() const;
    QUrl startUrl() const;
    double zoomFactor() const;
    QString styleScript() const;
    QString userScript() const;
    QString pageColorScript() const;

    Q_INVOKABLE void rememberZoomFactor(double zoomFactor);
    Q_INVOKABLE void clearBrowsingData();
    Q_INVOKABLE bool staysInsidePanel(const QUrl &target) const;

Q_SIGNALS:
    void providerChanged();
    void scriptsChanged();
    void downloadSaved(const QString &filePath);
    void pageStorageClearRequested(const QString &script);

private:
    void configureStorage();
    void configureUserAgent();
    void refreshScripts();
    void saveDownload(QQuickWebEngineDownloadRequest *download);

    Provider m_provider;
    OverrideFiles &m_files;
    ProviderRegistry &m_registry;
    QQuickWebEngineProfile *m_profile;
    NavigationPolicy m_navigationPolicy;
    OverrideReloader m_reloader;
    QString m_defaultUserAgent;
    QString m_styleScript;
    QString m_userScript;
};
