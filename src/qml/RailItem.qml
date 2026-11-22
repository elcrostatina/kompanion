// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.AbstractButton {
    id: item

    required property string providerId
    required property string name
    required property string iconName
    required property int index

    property bool selected: false
    property bool expanded: false
    property real labelReveal: expanded ? 1 : 0
    property url faviconUrl
    property int discSize: Kirigami.Units.gridUnit * 2
    property int itemPadding: Kirigami.Units.smallSpacing

    readonly property color accentColor: disc.tint

    width: ListView.view ? ListView.view.width : implicitWidth
    height: discSize + itemPadding * 2
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    Accessible.name: name
    Accessible.role: Accessible.Button

    QQC2.ToolTip.visible: hovered && !expanded
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    QQC2.ToolTip.text: i18n("%1 (Ctrl+%2)", name, index + 1)

    background: Rectangle {
        radius: Kirigami.Units.cornerRadius * 1.5
        color: "transparent"
        border.width: item.visualFocus ? 2 : 0
        border.color: item.accentColor
    }

    contentItem: Item {
        ProviderDisc {
            id: disc

            x: item.itemPadding
            anchors.verticalCenter: parent.verticalCenter
            width: item.discSize
            height: item.discSize
            name: item.name
            iconName: item.iconName
            faviconUrl: item.faviconUrl
            selected: item.selected
            hovered: item.hovered
        }

        QQC2.Label {
            anchors.left: disc.right
            anchors.leftMargin: Kirigami.Units.largeSpacing + (1 - item.labelReveal) * Kirigami.Units.largeSpacing
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: item.name
            elide: Text.ElideRight
            color: Kirigami.Theme.textColor
            font.bold: item.selected
            opacity: item.labelReveal
        }
    }
}
