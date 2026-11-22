// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/ChatListModel.h"

#include "core/ChatStore.h"

#include <QDateTime>

ChatListModel::ChatListModel(const ChatStore &store, QString providerId, QObject *parent)
    : QAbstractListModel(parent)
    , m_store(store)
    , m_providerId(std::move(providerId))
{
    reload();
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_chats.size());
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }
    const ChatSummary &chat = m_chats.at(index.row());
    switch (role) {
    case ChatIdRole:
        return chat.id;
    case TitleRole:
        return chat.title;
    case ModelRole:
        return chat.model;
    case UpdatedAtRole:
        return QDateTime::fromMSecsSinceEpoch(chat.updatedAt);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    return {
        {ChatIdRole, "chatId"},
        {TitleRole, "title"},
        {ModelRole, "model"},
        {UpdatedAtRole, "updatedAt"},
    };
}

void ChatListModel::reload(const QString &searchText)
{
    beginResetModel();
    m_chats = m_store.chats(m_providerId, searchText);
    endResetModel();
}
