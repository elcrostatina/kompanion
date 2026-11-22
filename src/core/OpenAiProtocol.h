// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ChatMessage.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

namespace OpenAiProtocol {

struct StreamEvent {
    enum class Kind {
        Delta,
        Done,
        Failure,
        Ignored,
    };

    Kind kind = Kind::Ignored;
    QString text;
};

QByteArray chatRequestBody(const QString &model, const QList<ChatMessage> &messages);
StreamEvent parseStreamEvent(const QByteArray &eventData);
QStringList parseModelIds(const QByteArray &responseBody);
QString parseErrorMessage(const QByteArray &responseBody);

}
