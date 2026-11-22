// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/PanelSide.h"

#include <QObject>
#include <QPropertyAnimation>
#include <QQmlEngine>
#include <QString>

class AppSettings;
class PanelDocking;
struct PanelPlacement;
class QQuickWindow;

class PanelController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")

    Q_PROPERTY(int panelWidth READ panelWidth NOTIFY panelWidthChanged FINAL)
    Q_PROPERTY(qreal revealProgress READ revealProgress WRITE setRevealProgress NOTIFY revealProgressChanged FINAL)
    Q_PROPERTY(QString side READ side WRITE setSide NOTIFY sideChanged FINAL)
    Q_PROPERTY(bool floating READ isFloating WRITE setFloating NOTIFY floatingChanged FINAL)
    Q_PROPERTY(bool shown READ isShown NOTIFY shownChanged FINAL)
    Q_PROPERTY(QString notice READ notice NOTIFY noticeChanged FINAL)

public:
    PanelController(AppSettings &settings, PanelDocking &docking, QObject *parent = nullptr);

    void attachWindow(QQuickWindow *window);

    int panelWidth() const;
    bool isFloating() const;
    void setFloating(bool floating);
    QString side() const;
    void setSide(const QString &sideName);
    qreal revealProgress() const;
    bool isShown() const;
    QString notice() const;

    void setRevealProgress(qreal progress);
    void postNotice(const QString &message);

    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void toggle();
    void showSettings();
    Q_INVOKABLE void resizeTo(int width);
    Q_INVOKABLE void persistWidth();
    Q_INVOKABLE void dismissNotice();

Q_SIGNALS:
    void panelWidthChanged();
    void sideChanged();
    void floatingChanged();
    void revealProgressChanged();
    void shownChanged();
    void noticeChanged();
    void openSettingsRequested();

private:
    void animateRevealTo(qreal target);
    void hideWindowWhenFullyConcealed();
    void setShown(bool shown);
    PanelPlacement placement() const;

    AppSettings &m_settings;
    PanelDocking &m_docking;
    QQuickWindow *m_window = nullptr;
    QPropertyAnimation m_revealAnimation;
    int m_panelWidth;
    PanelSide m_side;
    bool m_floating;
    qreal m_revealProgress = 0.0;
    bool m_shown = false;
    QString m_notice;
};
