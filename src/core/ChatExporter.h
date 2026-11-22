// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/ChatRecords.h"

#include <QByteArray>

#include <optional>

namespace ChatExporter {

QString toMarkdown(const ChatTranscript &transcript);
QByteArray toJson(const ChatTranscript &transcript);
std::optional<ChatTranscript> fromJson(const QByteArray &json);

}
