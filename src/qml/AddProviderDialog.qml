// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: dialog

    required property var editor
    property string kind: "web"

    readonly property bool isWeb: kind === "web"

    function openFor(providerKind) {
        kind = providerKind
        nameField.clear()
        addressField.clear()
        iconField.clear()
        open()
    }

    title: isWeb ? i18n("Add web provider") : i18n("Add local provider")
    subtitle: isWeb ? i18n("Any AI chat web app that works in a browser.") : i18n("A server with an OpenAI-compatible API, on this computer or your network.")
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
    onAccepted: editor.addProvider(kind, nameField.text, addressField.text, iconField.text)

    Kirigami.FormLayout {
        wideMode: false
        QQC2.TextField {
            Layout.fillWidth: true
            id: nameField

            Kirigami.FormData.label: i18n("Name:")
            Accessible.name: i18n("Provider name")
        }

        QQC2.TextField {
            Layout.fillWidth: true
            id: addressField

            Kirigami.FormData.label: dialog.isWeb ? i18n("Address:") : i18n("API address:")
            placeholderText: dialog.isWeb ? "https://example.com" : "http://localhost:1234/v1"
            inputMethodHints: Qt.ImhUrlCharactersOnly
            Accessible.name: Kirigami.FormData.label
        }

        QQC2.TextField {
            Layout.fillWidth: true
            id: iconField

            Kirigami.FormData.label: i18n("Icon:")
            placeholderText: i18n("Optional icon name or file")
            Accessible.name: i18n("Icon")
        }
    }
}
