// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/OverrideFiles.h"

#include <QDir>
#include <QFile>
#include <QSaveFile>

namespace {

constexpr QLatin1StringView DefaultsResourcePrefix{":/kompanion/overrides/"};
constexpr QLatin1StringView GenericDefaultName{"generic"};
constexpr QLatin1StringView CssExtension{".css"};
constexpr QLatin1StringView JavaScriptExtension{".js"};

QString defaultResourcePath(const QString &name)
{
    return QString(DefaultsResourcePrefix) + name + QString(CssExtension);
}

}

OverrideFiles::OverrideFiles(QString directory)
    : m_directory(std::move(directory))
{
}

QString OverrideFiles::directory() const
{
    return m_directory;
}

QString OverrideFiles::cssPath(const QString &providerId) const
{
    return QDir(m_directory).filePath(providerId + QString(CssExtension));
}

QString OverrideFiles::javaScriptPath(const QString &providerId) const
{
    return QDir(m_directory).filePath(providerId + QString(JavaScriptExtension));
}

QString OverrideFiles::readCss(const QString &providerId) const
{
    return readFile(cssPath(providerId));
}

QString OverrideFiles::readJavaScript(const QString &providerId) const
{
    return readFile(javaScriptPath(providerId));
}

void OverrideFiles::ensureCssExists(const QString &providerId) const
{
    if (!QFile::exists(cssPath(providerId))) {
        resetCss(providerId);
    }
}

void OverrideFiles::resetCss(const QString &providerId) const
{
    writeCss(providerId, defaultCssFor(providerId));
}

QString OverrideFiles::defaultCssFor(const QString &providerId)
{
    const QString specific = readFile(defaultResourcePath(providerId));
    return specific.isEmpty() ? readFile(defaultResourcePath(QString(GenericDefaultName))) : specific;
}

QString OverrideFiles::readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

bool OverrideFiles::writeCss(const QString &providerId, const QString &content) const
{
    if (!QDir().mkpath(m_directory)) {
        return false;
    }
    QSaveFile file(cssPath(providerId));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(content.toUtf8());
    return file.commit();
}
