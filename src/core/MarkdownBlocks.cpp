// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/MarkdownBlocks.h"

#include <optional>

namespace {

constexpr QLatin1StringView BacktickFence{"```"};
constexpr QLatin1StringView TildeFence{"~~~"};

struct Fence {
    QString marker;
    QString language;
};

std::optional<Fence> openingFence(const QString &line)
{
    const QString trimmed = line.trimmed();
    for (const QLatin1StringView marker : {BacktickFence, TildeFence}) {
        if (trimmed.startsWith(marker)) {
            QString run = QString(marker);
            qsizetype position = marker.size();
            while (position < trimmed.size() && trimmed.at(position) == run.at(0)) {
                run.append(run.at(0));
                ++position;
            }
            const QString info = trimmed.mid(position).trimmed();
            return Fence{run, info.section(QLatin1Char(' '), 0, 0)};
        }
    }
    return std::nullopt;
}

bool closesFence(const QString &line, const QString &marker)
{
    const QString trimmed = line.trimmed();
    return trimmed.startsWith(marker) && trimmed.count(marker.at(0)) == trimmed.size();
}

QString joined(const QStringList &lines)
{
    return lines.join(QLatin1Char('\n'));
}

void flushText(QStringList &textLines, QList<MarkdownBlock> &blocks)
{
    const QString text = joined(textLines).trimmed();
    if (!text.isEmpty()) {
        blocks.append({MarkdownBlock::Kind::Text, text, QString()});
    }
    textLines.clear();
}

}

QList<MarkdownBlock> MarkdownBlocks::split(const QString &markdown)
{
    QList<MarkdownBlock> blocks;
    QStringList textLines;
    QStringList codeLines;
    std::optional<Fence> openFence;

    for (const QString &line : markdown.split(QLatin1Char('\n'))) {
        if (openFence) {
            if (closesFence(line, openFence->marker)) {
                blocks.append({MarkdownBlock::Kind::Code, joined(codeLines), openFence->language});
                codeLines.clear();
                openFence.reset();
            } else {
                codeLines.append(line);
            }
            continue;
        }
        const std::optional<Fence> fence = openingFence(line);
        if (fence) {
            flushText(textLines, blocks);
            openFence = fence;
        } else {
            textLines.append(line);
        }
    }
    if (openFence) {
        blocks.append({MarkdownBlock::Kind::Code, joined(codeLines), openFence->language});
    }
    flushText(textLines, blocks);
    return blocks;
}
