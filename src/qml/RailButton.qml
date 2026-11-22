// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.AbstractButton {
    id: button

    property string iconName: ""
    property bool accent: false
    property bool active: false
    property bool expanded: false
    property real labelReveal: expanded ? 1 : 0
    property real iconRotation: 0
    property int discSize: Kirigami.Units.gridUnit * 2
    property int itemPadding: Kirigami.Units.smallSpacing

    readonly property bool filled: accent || active
    readonly property real cornerFactor: 0.32
    readonly property real restingFillAlpha: 0.16
    readonly property real hoverFillAlpha: 0.28
    readonly property real rowHoverAlpha: 0.08
    readonly property real borderAlpha: 0.4
    readonly property real pressedScale: 0.94

    implicitHeight: discSize + itemPadding * 2
    implicitWidth: discSize + itemPadding * 2
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    Accessible.name: text
    Accessible.role: Accessible.Button

    QQC2.ToolTip.visible: hovered && !expanded
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    QQC2.ToolTip.text: text

    background: Rectangle {
        radius: Kirigami.Units.cornerRadius * 1.5
        color: button.hovered ? Qt.alpha(Kirigami.Theme.highlightColor, button.rowHoverAlpha) : "transparent"
        border.width: button.visualFocus ? 2 : 0
        border.color: Kirigami.Theme.highlightColor

        Behavior on color {
            ColorAnimation {
                duration: Kirigami.Units.shortDuration
            }
        }
    }

    contentItem: Item {
        Rectangle {
            id: tile

            x: button.itemPadding
            anchors.verticalCenter: parent.verticalCenter
            width: button.discSize
            height: button.discSize
            radius: width * button.cornerFactor
            scale: button.pressed ? button.pressedScale : 1
            color: button.filled ? Kirigami.Theme.highlightColor : Qt.alpha(Kirigami.Theme.highlightColor, button.hovered ? button.hoverFillAlpha : button.restingFillAlpha)
            border.width: button.filled ? 0 : 1
            border.color: Qt.alpha(Kirigami.Theme.highlightColor, button.borderAlpha)

            Behavior on color {
                ColorAnimation {
                    duration: Kirigami.Units.shortDuration
                }
            }

            Behavior on scale {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration
                }
            }

            Kirigami.Icon {
                anchors.centerIn: parent
                width: Kirigami.Units.iconSizes.smallMedium
                height: width
                source: button.iconName
                rotation: button.iconRotation
                color: button.filled ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.highlightColor
                isMask: true
            }
        }

        QQC2.Label {
            anchors.left: tile.right
            anchors.leftMargin: Kirigami.Units.largeSpacing + (1 - button.labelReveal) * Kirigami.Units.largeSpacing
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: button.text
            elide: Text.ElideRight
            font.bold: button.active
            opacity: button.labelReveal
        }
    }
}
