// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/SseParser.h"

namespace {

constexpr char LineFeed = '\n';
constexpr char CarriageReturn = '\r';
constexpr char Space = ' ';
const QByteArray DataFieldPrefix = QByteArrayLiteral("data:");

}

QList<QByteArray> SseParser::feed(const QByteArray &chunk)
{
    QList<QByteArray> completedEvents;
    m_buffer.append(chunk);
    qsizetype lineEnd = m_buffer.indexOf(LineFeed);
    while (lineEnd >= 0) {
        consumeLine(m_buffer.left(lineEnd), completedEvents);
        m_buffer.remove(0, lineEnd + 1);
        lineEnd = m_buffer.indexOf(LineFeed);
    }
    return completedEvents;
}

QList<QByteArray> SseParser::finish()
{
    QList<QByteArray> completedEvents;
    if (!m_buffer.isEmpty()) {
        consumeLine(m_buffer, completedEvents);
        m_buffer.clear();
    }
    completePendingEvent(completedEvents);
    return completedEvents;
}

void SseParser::consumeLine(const QByteArray &rawLine, QList<QByteArray> &completedEvents)
{
    QByteArray line = rawLine;
    if (line.endsWith(CarriageReturn)) {
        line.chop(1);
    }
    if (line.isEmpty()) {
        completePendingEvent(completedEvents);
    } else if (line.startsWith(DataFieldPrefix)) {
        appendDataLine(line);
    }
}

void SseParser::appendDataLine(const QByteArray &line)
{
    QByteArray value = line.mid(DataFieldPrefix.size());
    if (value.startsWith(Space)) {
        value.remove(0, 1);
    }
    if (m_hasPendingData) {
        m_pendingData.append(LineFeed);
    }
    m_pendingData.append(value);
    m_hasPendingData = true;
}

void SseParser::completePendingEvent(QList<QByteArray> &completedEvents)
{
    if (m_hasPendingData) {
        completedEvents.append(m_pendingData);
    }
    m_pendingData.clear();
    m_hasPendingData = false;
}
