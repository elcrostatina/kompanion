// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/FramelessDocking.h"

#include "core/PanelLimits.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

namespace {

constexpr Qt::WindowFlags PanelWindowFlags = Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool;

}

void FramelessDocking::attach(QWindow *window, const PanelPlacement &placement)
{
    window->setFlags(PanelWindowFlags);
    place(window, placement);
}

void FramelessDocking::place(QWindow *window, const PanelPlacement &placement)
{
    QScreen *screen = screenForPanel();
    const int gap = placement.floating ? PanelLimits::FloatingGap : 0;
    const QRect available = screen->geometry().adjusted(gap, gap, -gap, -gap);
    const int left = placement.side == PanelSide::Right ? available.right() + 1 - placement.width : available.left();
    window->setScreen(screen);
    window->setGeometry(QRect(QPoint(left, available.top()), QSize(placement.width, available.height())));
}

QScreen *FramelessDocking::screenForPanel()
{
    QScreen *screenUnderCursor = QGuiApplication::screenAt(QCursor::pos());
    return screenUnderCursor ? screenUnderCursor : QGuiApplication::primaryScreen();
}
