// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.ToolBar {
    id: composer

    required property bool busy

    readonly property int maximumVisibleLines: 8

    signal sendRequested(string text)
    signal stopRequested()

    function submit() {
        const message = input.text.trim()
        if (message.length > 0 && !busy) {
            sendRequested(message)
            input.clear()
        }
    }

    function focusInput() {
        input.forceActiveFocus()
    }

    position: QQC2.ToolBar.Footer

    RowLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.maximumHeight: input.font.pixelSize * composer.maximumVisibleLines * 1.6

            QQC2.TextArea {
                id: input

                placeholderText: i18n("Message… (Enter to send, Shift+Enter for a new line)")
                wrapMode: TextEdit.Wrap
                Accessible.name: i18n("Message")

                Keys.onPressed: event => {
                    const isEnter = event.key === Qt.Key_Return || event.key === Qt.Key_Enter
                    if (isEnter && !(event.modifiers & Qt.ShiftModifier)) {
                        composer.submit()
                        event.accepted = true
                    }
                }
            }
        }

        QQC2.Button {
            icon.name: composer.busy ? "media-playback-stop" : "document-send"
            text: composer.busy ? i18n("Stop") : i18n("Send")
            display: QQC2.AbstractButton.IconOnly
            enabled: composer.busy || input.text.trim().length > 0
            Accessible.name: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: text
            onClicked: composer.busy ? composer.stopRequested() : composer.submit()
        }
    }
}
