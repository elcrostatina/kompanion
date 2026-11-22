// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ChatRecords.h"

#include <QAbstractListModel>
#include <QQmlEngine>

class ChatStore;

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by ChatController")

public:
    enum Role {
        ChatIdRole = Qt::UserRole + 1,
        TitleRole,
        ModelRole,
        UpdatedAtRole,
    };

    ChatListModel(const ChatStore &store, QString providerId, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reload(const QString &searchText = QString());

private:
    const ChatStore &m_store;
    QString m_providerId;
    QList<ChatSummary> m_chats;
};
