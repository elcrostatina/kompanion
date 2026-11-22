// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class OverrideFiles
{
public:
    explicit OverrideFiles(QString directory);

    QString directory() const;
    QString cssPath(const QString &providerId) const;
    QString javaScriptPath(const QString &providerId) const;

    QString readCss(const QString &providerId) const;
    QString readJavaScript(const QString &providerId) const;

    void ensureCssExists(const QString &providerId) const;
    void resetCss(const QString &providerId) const;

private:
    static QString defaultCssFor(const QString &providerId);
    static QString readFile(const QString &path);
    bool writeCss(const QString &providerId, const QString &content) const;

    QString m_directory;
};
