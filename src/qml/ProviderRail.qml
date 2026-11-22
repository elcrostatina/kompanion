// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ShadowedRectangle {
    id: rail

    required property var providers
    required property var favicons
    required property string currentId
    required property int currentIndex

    property bool expanded: false
    property bool mirrored: false
    property bool settingsOpen: false
    property real expandProgress: expanded ? 1 : 0

    readonly property int discSize: Kirigami.Units.gridUnit * 2
    readonly property int railPadding: Kirigami.Units.smallSpacing
    readonly property int itemPadding: Kirigami.Units.smallSpacing
    readonly property int collapsedWidth: discSize + itemPadding * 2 + railPadding * 2
    readonly property int expandedWidth: Math.max(collapsedWidth, Math.min(Kirigami.Units.gridUnit * 11, (parent ? parent.width : width) * expandedWidthShare))
    readonly property int accentBarWidth: 3
    readonly property int expandDurationMs: Kirigami.Units.longDuration * 1.4
    readonly property int collapseDurationMs: Kirigami.Units.longDuration
    readonly property real labelDelay: 0.25
    readonly property real labelSpan: 0.5
    readonly property real staggerStep: 0.03
    readonly property real halfTurn: 180
    readonly property real borderAlpha: 0.14
    readonly property real shadowAlpha: 0.35
    readonly property real sectionOpacity: 0.55
    readonly property real sectionSpacing: 1.5
    readonly property real highlightAlpha: 0.1
    readonly property real expandedWidthShare: 0.7

    signal providerSelected(string providerId)
    signal settingsRequested()
    signal expandToggled()

    function sectionTitle(kind) {
        return (kind === "web" ? i18n("Web") : i18n("Local")).toUpperCase()
    }

    function reveal(order) {
        const progress = (expandProgress - labelDelay - order * staggerStep) / labelSpan
        return Math.max(0, Math.min(1, progress))
    }

    Behavior on expandProgress {
        NumberAnimation {
            duration: rail.expanded ? rail.expandDurationMs : rail.collapseDurationMs
            easing.type: rail.expanded ? Easing.OutCubic : Easing.InOutCubic
        }
    }

    width: collapsedWidth + (expandedWidth - collapsedWidth) * expandProgress
    radius: Kirigami.Units.cornerRadius * 2
    color: Kirigami.Theme.alternateBackgroundColor
    border.width: 1
    border.color: Qt.alpha(Kirigami.Theme.textColor, borderAlpha)
    shadow.size: Kirigami.Units.gridUnit * expandProgress
    shadow.color: Qt.rgba(0, 0, 0, shadowAlpha)
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: rail.railPadding
        spacing: Kirigami.Units.smallSpacing

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: rail.discSize + rail.itemPadding * 2

            ProviderDisc {
                id: logo

                x: rail.itemPadding
                anchors.verticalCenter: parent.verticalCenter
                width: rail.discSize
                height: rail.discSize
                glyph: "K"
                tint: Kirigami.Theme.highlightColor
                selected: true
            }

            QQC2.Label {
                anchors.left: logo.right
                anchors.leftMargin: Kirigami.Units.largeSpacing + (1 - opacity) * Kirigami.Units.largeSpacing
                anchors.verticalCenter: parent.verticalCenter
                text: "Kompanion"
                font.bold: true
                font.letterSpacing: 0.5
                opacity: rail.reveal(0)
            }
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: rail.providers
            currentIndex: rail.currentIndex
            interactive: contentHeight > height
            boundsBehavior: Flickable.StopAtBounds
            spacing: Kirigami.Units.smallSpacing
            highlightMoveDuration: Kirigami.Units.longDuration
            highlightResizeDuration: 0

            section.property: "kind"
            section.delegate: Item {
                id: sectionHeader

                required property string section

                width: list.width
                height: sectionLabel.implicitHeight + Kirigami.Units.smallSpacing * 2

                QQC2.Label {
                    id: sectionLabel

                    x: (sectionHeader.width - width) / 2 * (1 - rail.expandProgress) + rail.itemPadding * rail.expandProgress
                    anchors.verticalCenter: parent.verticalCenter
                    text: rail.sectionTitle(sectionHeader.section)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    font.letterSpacing: rail.sectionSpacing
                    opacity: rail.sectionOpacity
                }
            }

            QQC2.ScrollBar.vertical: QQC2.ScrollBar {
                policy: QQC2.ScrollBar.AsNeeded
            }

            highlight: Rectangle {
                readonly property color accent: list.currentItem ? list.currentItem.accentColor : Kirigami.Theme.highlightColor

                radius: Kirigami.Units.cornerRadius * 1.5
                color: Qt.alpha(accent, rail.highlightAlpha)

                Behavior on color {
                    ColorAnimation {
                        duration: Kirigami.Units.longDuration
                    }
                }

                Rectangle {
                    x: rail.mirrored ? parent.width - width : 0
                    anchors.verticalCenter: parent.verticalCenter
                    width: rail.accentBarWidth
                    height: parent.height * 0.6
                    radius: width / 2
                    color: parent.accent
                }
            }

            delegate: RailItem {
                selected: providerId === rail.currentId
                expanded: rail.expanded
                labelReveal: rail.reveal(index + 1)
                faviconUrl: {
                    rail.favicons.revision
                    return rail.favicons.urlFor(providerId)
                }
                discSize: rail.discSize
                itemPadding: rail.itemPadding
                onClicked: rail.providerSelected(providerId)
            }
        }

        RailButton {
            Layout.fillWidth: true
            accent: true
            expanded: rail.expanded
            labelReveal: rail.reveal(list.count + 1)
            iconRotation: (rail.mirrored ? rail.halfTurn : 0) + rail.halfTurn * rail.expandProgress
            discSize: rail.discSize
            itemPadding: rail.itemPadding
            iconName: "go-next"
            text: rail.expanded ? i18n("Collapse") : i18n("Expand")
            onClicked: rail.expandToggled()
        }

        RailButton {
            Layout.fillWidth: true
            active: rail.settingsOpen
            expanded: rail.expanded
            labelReveal: rail.reveal(list.count + 2)
            discSize: rail.discSize
            itemPadding: rail.itemPadding
            iconName: "configure"
            text: i18n("Settings")
            onClicked: rail.settingsRequested()
        }
    }
}
