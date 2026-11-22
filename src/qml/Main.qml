// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Effects
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import io.github.elcrostatina.kompanion

QQC2.ApplicationWindow {
    id: window

    required property PanelController panel
    required property var enabledProviders
    required property var providerSelection
    required property var webProfiles
    required property var chatControllers

    required property var allProviders
    required property var preferences
    required property var providerEditor

    readonly property int maximumShortcutProviders: 9

    readonly property int resizeHandleWidth: Kirigami.Units.smallSpacing
    readonly property bool onRightEdge: panel.side === "right"
    readonly property bool floating: panel.floating
    readonly property int floatingCornerRadius: Kirigami.Units.cornerRadius * 2
    readonly property int cornerRadius: floating ? floatingCornerRadius : 0
    readonly property real floatingBorderOpacity: 0.25
    readonly property int pageSlideDistance: Kirigami.Units.gridUnit * 1.5
    readonly property int cardGap: Kirigami.Units.smallSpacing + 2
    readonly property int dockedRailMargin: Kirigami.Units.smallSpacing
    readonly property int railMargin: floating ? 0 : dockedRailMargin
    readonly property int contentInset: rail.collapsedWidth + (floating ? cardGap : dockedRailMargin * 2)

    function showProviders() {
        if (pages.depth > 1) {
            pages.pop(null)
        }
    }

    function toggleSettings() {
        if (pages.depth > 1) {
            showProviders()
        } else {
            pages.openSettings()
        }
    }

    title: "Kompanion"
    width: panel.panelWidth
    height: Screen.height
    visible: false
    color: "transparent"

    Shortcut {
        sequence: "Esc"
        onActivated: window.panel.hide()
    }

    Instantiator {
        model: window.maximumShortcutProviders

        delegate: Shortcut {
            required property int index

            sequence: "Ctrl+" + (index + 1)
            onActivated: {
                window.showProviders()
                window.providerSelection.selectIndex(index)
            }
        }
    }

    background: null

    Connections {
        target: window.panel

        function onOpenSettingsRequested() {
            pages.openSettings()
        }
    }

    Item {
        anchors.fill: parent

        Item {
            id: sheet

            width: window.panel.panelWidth
            height: parent.height
            x: window.onRightEdge ? parent.width - width * window.panel.revealProgress : -width * (1 - window.panel.revealProgress)

            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false

            Rectangle {
                anchors.fill: parent
                visible: !window.floating
                color: Kirigami.Theme.backgroundColor
            }

            Kirigami.Separator {
                visible: !window.floating
                x: window.onRightEdge ? 0 : parent.width - width
                height: parent.height
            }

            Rectangle {
                id: contentCard

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: window.onRightEdge ? 0 : window.contentInset
                anchors.rightMargin: window.onRightEdge ? window.contentInset : 0
                radius: window.cornerRadius
                color: Kirigami.Theme.backgroundColor

                Rectangle {
                    id: cornerMask

                    width: contentCard.width
                    height: contentCard.height
                    radius: window.cornerRadius
                    visible: false
                    layer.enabled: true
                }

                Item {
                    id: clippedContent

                    anchors.fill: parent
                    layer.enabled: window.floating
                    layer.effect: MultiEffect {
                        maskEnabled: true
                        maskSource: cornerMask
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        Kirigami.InlineMessage {
                            Layout.fillWidth: true
                            Layout.margins: Kirigami.Units.smallSpacing
                            type: Kirigami.MessageType.Warning
                            visible: window.panel.notice.length > 0
                            text: window.panel.notice
                            showCloseButton: true
                            onVisibleChanged: if (!visible) window.panel.dismissNotice()
                        }

                        QQC2.StackView {
                            id: pages

                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            initialItem: ProvidersPage {
                                providers: window.enabledProviders
                                providerSelection: window.providerSelection
                                webProfiles: window.webProfiles
                                chatControllers: window.chatControllers
                            }

                            pushEnter: Transition {
                                ParallelAnimation {
                                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                    NumberAnimation { property: "x"; from: window.pageSlideDistance; to: 0; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                }
                            }
                            pushExit: Transition {
                                ParallelAnimation {
                                    NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                    NumberAnimation { property: "x"; from: 0; to: -window.pageSlideDistance; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                }
                            }
                            popEnter: Transition {
                                ParallelAnimation {
                                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                    NumberAnimation { property: "x"; from: -window.pageSlideDistance; to: 0; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                }
                            }
                            popExit: Transition {
                                ParallelAnimation {
                                    NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                    NumberAnimation { property: "x"; from: 0; to: window.pageSlideDistance; duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic }
                                }
                            }

                            function openSettings() {
                                if (depth === 1) {
                                    push(Qt.resolvedUrl("SettingsPage.qml"), {
                                        panel: window.panel,
                                        preferences: window.preferences,
                                        providerEditor: window.providerEditor,
                                        allProviders: window.allProviders
                                    })
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    visible: window.floating
                    radius: window.cornerRadius
                    color: "transparent"
                    border.width: 1
                    border.color: Qt.alpha(Kirigami.Theme.textColor, window.floatingBorderOpacity)
                }
            }

            ProviderRail {
                id: rail

                z: 1
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: window.onRightEdge ? undefined : parent.left
                anchors.right: window.onRightEdge ? parent.right : undefined
                anchors.margins: window.railMargin
                providers: window.enabledProviders
                currentId: window.providerSelection.currentId
                currentIndex: window.providerSelection.currentIndex
                expanded: window.preferences.sidebarExpanded
                mirrored: window.onRightEdge
                settingsOpen: pages.depth > 1
                onProviderSelected: providerId => {
                    window.showProviders()
                    window.providerSelection.selectId(providerId)
                }
                onSettingsRequested: window.toggleSettings()
                onExpandToggled: window.preferences.sidebarExpanded = !window.preferences.sidebarExpanded
            }

            MouseArea {
                id: resizeHandle

                z: 2
                x: window.onRightEdge ? 0 : parent.width - width
                width: window.resizeHandleWidth
                height: parent.height
                cursorShape: Qt.SizeHorCursor
                acceptedButtons: Qt.LeftButton

                onPositionChanged: mouse => {
                    if (pressed) {
                        const edgeOffset = mapToItem(sheet, mouse.x, 0).x
                        window.panel.resizeTo(window.onRightEdge ? sheet.width - edgeOffset : edgeOffset)
                    }
                }
                onReleased: window.panel.persistWidth()
            }
        }
    }
}
