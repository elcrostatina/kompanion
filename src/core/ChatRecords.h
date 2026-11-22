// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QString>

struct ChatSummary {
    qint64 id = 0;
    QString providerId;
    QString title;
    QString model;
    QString systemPrompt;
    qint64 createdAt = 0;
    qint64 updatedAt = 0;
};

struct StoredMessage {
    qint64 id = 0;
    qint64 chatId = 0;
    QString role;
    QString content;
    qint64 createdAt = 0;
};

struct ChatTranscript {
    ChatSummary chat;
    QList<StoredMessage> messages;
};
