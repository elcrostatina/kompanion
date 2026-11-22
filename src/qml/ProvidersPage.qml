// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

Item {
    id: page

    required property var providers
    required property var providerSelection
    required property var webProfiles
    required property var chatControllers

    ProviderStack {
        anchors.fill: parent
        providers: page.providers
        webProfiles: page.webProfiles
        chatControllers: page.chatControllers
        currentId: page.providerSelection.currentId
        currentIndex: page.providerSelection.currentIndex
    }
}
