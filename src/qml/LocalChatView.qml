// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property var chatController

    property bool showingChatList: false

    Component.onCompleted: chatController.refreshModels()

    StackLayout {
        anchors.fill: parent
        currentIndex: root.showingChatList ? 1 : 0

        ConversationPage {
            controller: root.chatController
            onChatListRequested: root.showingChatList = true
        }

        ChatListPage {
            controller: root.chatController
            onChatChosen: root.showingChatList = false
        }
    }
}
