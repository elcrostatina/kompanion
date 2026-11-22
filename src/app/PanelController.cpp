// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/PanelController.h"

#include "app/PanelDocking.h"
#include "core/AppSettings.h"
#include "core/PanelLimits.h"

#include <QEasingCurve>
#include <QQuickWindow>

namespace {

constexpr int SlideDurationMs = 150;
constexpr qreal FullyConcealed = 0.0;
constexpr qreal FullyRevealed = 1.0;

}

PanelController::PanelController(AppSettings &settings, PanelDocking &docking, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_docking(docking)
    , m_revealAnimation(this, "revealProgress")
    , m_panelWidth(settings.panelWidth())
    , m_side(settings.panelSide())
    , m_floating(settings.panelFloating())
{
    m_revealAnimation.setDuration(SlideDurationMs);
    m_revealAnimation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_revealAnimation, &QAbstractAnimation::finished, this, &PanelController::hideWindowWhenFullyConcealed);
}

void PanelController::attachWindow(QQuickWindow *window)
{
    m_window = window;
    m_docking.attach(m_window, placement());
}

int PanelController::panelWidth() const
{
    return m_panelWidth;
}

bool PanelController::isFloating() const
{
    return m_floating;
}

void PanelController::setFloating(bool floating)
{
    if (floating == m_floating) {
        return;
    }
    m_floating = floating;
    m_settings.setPanelFloating(m_floating);
    if (m_window) {
        m_docking.place(m_window, placement());
    }
    Q_EMIT floatingChanged();
}

QString PanelController::side() const
{
    return panelSideName(m_side);
}

void PanelController::setSide(const QString &sideName)
{
    const PanelSide requested = panelSideFromName(sideName);
    if (requested == m_side) {
        return;
    }
    m_side = requested;
    m_settings.setPanelSide(m_side);
    if (m_window) {
        m_docking.place(m_window, placement());
    }
    Q_EMIT sideChanged();
}

qreal PanelController::revealProgress() const
{
    return m_revealProgress;
}

bool PanelController::isShown() const
{
    return m_shown;
}

QString PanelController::notice() const
{
    return m_notice;
}

void PanelController::setRevealProgress(qreal progress)
{
    if (qFuzzyCompare(m_revealProgress, progress)) {
        return;
    }
    m_revealProgress = progress;
    Q_EMIT revealProgressChanged();
}

void PanelController::postNotice(const QString &message)
{
    m_notice = message;
    Q_EMIT noticeChanged();
}

void PanelController::dismissNotice()
{
    postNotice(QString());
}

void PanelController::show()
{
    if (!m_window) {
        return;
    }
    setShown(true);
    m_docking.place(m_window, placement());
    m_window->show();
    m_window->raise();
    m_window->requestActivate();
    animateRevealTo(FullyRevealed);
}

void PanelController::hide()
{
    if (!m_window) {
        return;
    }
    setShown(false);
    animateRevealTo(FullyConcealed);
}

void PanelController::toggle()
{
    if (m_shown) {
        hide();
    } else {
        show();
    }
}

void PanelController::showSettings()
{
    show();
    Q_EMIT openSettingsRequested();
}

void PanelController::resizeTo(int width)
{
    const int clampedWidth = PanelLimits::clampWidth(width);
    if (clampedWidth == m_panelWidth) {
        return;
    }
    m_panelWidth = clampedWidth;
    if (m_window) {
        m_docking.place(m_window, placement());
    }
    Q_EMIT panelWidthChanged();
}

void PanelController::persistWidth()
{
    m_settings.setPanelWidth(m_panelWidth);
}

void PanelController::animateRevealTo(qreal target)
{
    m_revealAnimation.stop();
    m_revealAnimation.setStartValue(m_revealProgress);
    m_revealAnimation.setEndValue(target);
    m_revealAnimation.start();
}

void PanelController::hideWindowWhenFullyConcealed()
{
    if (!m_shown && m_window) {
        m_window->hide();
    }
}

PanelPlacement PanelController::placement() const
{
    return {m_panelWidth, m_side, m_floating};
}

void PanelController::setShown(bool shown)
{
    if (m_shown == shown) {
        return;
    }
    m_shown = shown;
    Q_EMIT shownChanged();
}
