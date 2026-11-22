// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ChatRecords.h"
#include "core/MarkdownBlocks.h"

#include <QAbstractListModel>
#include <QQmlEngine>

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by ChatController")

public:
    enum Role {
        MessageIdRole = Qt::UserRole + 1,
        AuthorRole,
        BlockKindRole,
        BlockTextRole,
        LanguageRole,
        FirstInMessageRole,
    };

    explicit MessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setMessages(const QList<StoredMessage> &messages);
    void appendMessage(qint64 id, const QString &author, const QString &content);
    void setLastContent(const QString &content);
    void removeLastMessage();

private:
    struct Entry {
        qint64 id = 0;
        QString author;
        QList<MarkdownBlock> blocks;
    };

    static QList<MarkdownBlock> blocksFor(const QString &content);
    qsizetype totalRows() const;
    qsizetype entryIndexOfRow(int row) const;
    qsizetype firstRowOfEntry(qsizetype entryIndex) const;
    void replaceLastBlocks(const QList<MarkdownBlock> &newBlocks);

    QList<Entry> m_entries;
};
