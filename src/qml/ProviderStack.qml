// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: stack

    required property var providers
    required property var webProfiles
    required property var chatControllers
    required property string currentId
    required property int currentIndex

    readonly property string webKind: "web"
    readonly property int slideDistance: Kirigami.Units.gridUnit * 1.5
    readonly property real hiddenOpacity: 0
    readonly property real visibleThreshold: 0.01

    Repeater {
        model: stack.providers

        delegate: Loader {
            id: page

            required property string providerId
            required property string kind
            required property int index

            readonly property bool isCurrent: index === stack.currentIndex

            property bool everSelected: false

            width: stack.width
            height: stack.height
            active: everSelected
            enabled: isCurrent
            opacity: isCurrent ? 1 : stack.hiddenOpacity
            y: isCurrent ? 0 : (index < stack.currentIndex ? -stack.slideDistance : stack.slideDistance)
            visible: opacity > stack.visibleThreshold
            sourceComponent: kind === stack.webKind ? webView : localChat

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }

            Behavior on y {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }

            Component.onCompleted: everSelected = providerId === stack.currentId

            Connections {
                target: stack

                function onCurrentIdChanged() {
                    if (stack.currentId === page.providerId) {
                        page.everSelected = true
                    }
                }
            }

            Component {
                id: webView

                WebProviderView {
                    webProfile: stack.webProfiles.profileFor(page.providerId)
                }
            }

            Component {
                id: localChat

                LocalChatView {
                    chatController: stack.chatControllers.controllerFor(page.providerId)
                }
            }
        }
    }
}
