// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QString>

struct MarkdownBlock {
    enum class Kind {
        Text,
        Code,
    };

    Kind kind = Kind::Text;
    QString text;
    QString language;

    bool operator==(const MarkdownBlock &other) const = default;
};

namespace MarkdownBlocks {

QList<MarkdownBlock> split(const QString &markdown);

}
