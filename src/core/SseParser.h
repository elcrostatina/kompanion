// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArray>
#include <QList>

class SseParser
{
public:
    QList<QByteArray> feed(const QByteArray &chunk);
    QList<QByteArray> finish();

private:
    void consumeLine(const QByteArray &line, QList<QByteArray> &completedEvents);
    void appendDataLine(const QByteArray &line);
    void completePendingEvent(QList<QByteArray> &completedEvents);

    QByteArray m_buffer;
    QByteArray m_pendingData;
    bool m_hasPendingData = false;
};
