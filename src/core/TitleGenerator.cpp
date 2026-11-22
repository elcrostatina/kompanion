// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/TitleGenerator.h"

#include <QRegularExpression>

namespace {

constexpr int MaximumTitleLength = 48;
constexpr QChar Ellipsis{u'…'};

QString firstMeaningfulLine(const QString &message)
{
    const QStringList lines = message.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (!line.trimmed().isEmpty()) {
            return line;
        }
    }
    return QString();
}

QString withoutMarkdownDecoration(const QString &line)
{
    static const QRegularExpression decoration(QStringLiteral("[#*`>_~]+"));
    static const QRegularExpression repeatedSpace(QStringLiteral("\\s+"));
    QString cleaned = line;
    cleaned.remove(decoration);
    return cleaned.replace(repeatedSpace, QStringLiteral(" ")).trimmed();
}

}

QString TitleGenerator::fromFirstMessage(const QString &message)
{
    const QString title = withoutMarkdownDecoration(firstMeaningfulLine(message));
    if (title.size() <= MaximumTitleLength) {
        return title;
    }
    return title.left(MaximumTitleLength).trimmed() + Ellipsis;
}
