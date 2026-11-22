// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/MessageModel.h"

namespace {

constexpr QLatin1StringView TextKind{"text"};
constexpr QLatin1StringView CodeKind{"code"};

QString kindName(MarkdownBlock::Kind kind)
{
    return QString(kind == MarkdownBlock::Kind::Code ? CodeKind : TextKind);
}

}

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(totalRows());
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }
    const qsizetype entryIndex = entryIndexOfRow(index.row());
    const Entry &entry = m_entries.at(entryIndex);
    const qsizetype blockIndex = index.row() - firstRowOfEntry(entryIndex);
    const MarkdownBlock &block = entry.blocks.at(blockIndex);
    switch (role) {
    case MessageIdRole:
        return entry.id;
    case AuthorRole:
        return entry.author;
    case BlockKindRole:
        return kindName(block.kind);
    case BlockTextRole:
        return block.text;
    case LanguageRole:
        return block.language;
    case FirstInMessageRole:
        return blockIndex == 0;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        {MessageIdRole, "messageId"},
        {AuthorRole, "author"},
        {BlockKindRole, "blockKind"},
        {BlockTextRole, "blockText"},
        {LanguageRole, "language"},
        {FirstInMessageRole, "firstInMessage"},
    };
}

void MessageModel::setMessages(const QList<StoredMessage> &messages)
{
    beginResetModel();
    m_entries.clear();
    for (const StoredMessage &message : messages) {
        m_entries.append({message.id, message.role, blocksFor(message.content)});
    }
    endResetModel();
}

void MessageModel::appendMessage(qint64 id, const QString &author, const QString &content)
{
    const QList<MarkdownBlock> blocks = blocksFor(content);
    const int firstNewRow = static_cast<int>(totalRows());
    beginInsertRows(QModelIndex(), firstNewRow, firstNewRow + static_cast<int>(blocks.size()) - 1);
    m_entries.append({id, author, blocks});
    endInsertRows();
}

void MessageModel::setLastContent(const QString &content)
{
    if (!m_entries.isEmpty()) {
        replaceLastBlocks(blocksFor(content));
    }
}

void MessageModel::removeLastMessage()
{
    if (m_entries.isEmpty()) {
        return;
    }
    const int firstRow = static_cast<int>(firstRowOfEntry(m_entries.size() - 1));
    const int lastRow = firstRow + static_cast<int>(m_entries.constLast().blocks.size()) - 1;
    beginRemoveRows(QModelIndex(), firstRow, lastRow);
    m_entries.removeLast();
    endRemoveRows();
}

QList<MarkdownBlock> MessageModel::blocksFor(const QString &content)
{
    QList<MarkdownBlock> blocks = MarkdownBlocks::split(content);
    if (blocks.isEmpty()) {
        blocks.append({MarkdownBlock::Kind::Text, QString(), QString()});
    }
    return blocks;
}

qsizetype MessageModel::totalRows() const
{
    qsizetype rows = 0;
    for (const Entry &entry : m_entries) {
        rows += entry.blocks.size();
    }
    return rows;
}

qsizetype MessageModel::entryIndexOfRow(int row) const
{
    qsizetype firstRow = 0;
    for (qsizetype entryIndex = 0; entryIndex < m_entries.size(); ++entryIndex) {
        firstRow += m_entries.at(entryIndex).blocks.size();
        if (row < firstRow) {
            return entryIndex;
        }
    }
    return m_entries.size() - 1;
}

qsizetype MessageModel::firstRowOfEntry(qsizetype entryIndex) const
{
    qsizetype rows = 0;
    for (qsizetype index = 0; index < entryIndex; ++index) {
        rows += m_entries.at(index).blocks.size();
    }
    return rows;
}

void MessageModel::replaceLastBlocks(const QList<MarkdownBlock> &newBlocks)
{
    Entry &last = m_entries.last();
    const int firstRow = static_cast<int>(firstRowOfEntry(m_entries.size() - 1));
    const qsizetype oldCount = last.blocks.size();
    const qsizetype sharedCount = qMin(oldCount, newBlocks.size());

    for (qsizetype blockIndex = 0; blockIndex < sharedCount; ++blockIndex) {
        if (last.blocks.at(blockIndex) != newBlocks.at(blockIndex)) {
            last.blocks[blockIndex] = newBlocks.at(blockIndex);
            const QModelIndex changed = index(firstRow + static_cast<int>(blockIndex));
            Q_EMIT dataChanged(changed, changed);
        }
    }
    if (newBlocks.size() > oldCount) {
        beginInsertRows(QModelIndex(), firstRow + static_cast<int>(oldCount), firstRow + static_cast<int>(newBlocks.size()) - 1);
        last.blocks = newBlocks;
        endInsertRows();
    } else if (newBlocks.size() < oldCount) {
        beginRemoveRows(QModelIndex(), firstRow + static_cast<int>(newBlocks.size()), firstRow + static_cast<int>(oldCount) - 1);
        last.blocks = newBlocks;
        endRemoveRows();
    }
}
