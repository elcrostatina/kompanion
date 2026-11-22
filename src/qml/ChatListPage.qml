// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: page

    required property var controller

    signal chatChosen()

    spacing: 0

    QQC2.ToolBar {
        Layout.fillWidth: true

        RowLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            QQC2.ToolButton {
                icon.name: "go-previous"
                text: i18n("Back to chat")
                display: QQC2.AbstractButton.IconOnly
                Accessible.name: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                onClicked: page.chatChosen()
            }

            Kirigami.SearchField {
                id: searchField

                Layout.fillWidth: true
                placeholderText: i18n("Search chats…")
                onTextChanged: page.controller.search(text)
            }

            QQC2.ToolButton {
                icon.name: "list-add"
                text: i18n("New chat")
                display: QQC2.AbstractButton.IconOnly
                enabled: !page.controller.busy
                Accessible.name: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                onClicked: {
                    page.controller.newChat()
                    page.chatChosen()
                }
            }
        }
    }

    ListView {
        id: chatList

        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        model: page.controller.chats
        boundsBehavior: Flickable.StopAtBounds
        QQC2.ScrollBar.vertical: QQC2.ScrollBar {}

        delegate: QQC2.ItemDelegate {
            id: row

            required property var chatId
            required property string title
            required property string model
            required property date updatedAt

            width: ListView.view.width
            enabled: !page.controller.busy
            highlighted: chatId === page.controller.currentChatId
            text: title.length > 0 ? title : i18n("New chat")

            contentItem: ColumnLayout {
                spacing: 0

                QQC2.Label {
                    Layout.fillWidth: true
                    text: row.text
                    elide: Text.ElideRight
                    font.bold: row.highlighted
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: row.updatedAt.toLocaleString(Qt.locale(), Locale.ShortFormat) + (row.model.length > 0 ? " · " + row.model : "")
                    elide: Text.ElideRight
                    opacity: 0.7
                    font: Kirigami.Theme.smallFont
                }
            }

            onClicked: {
                page.controller.openChat(chatId)
                page.chatChosen()
            }

            QQC2.ToolButton {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                icon.name: "overflow-menu"
                text: i18n("Chat actions")
                display: QQC2.AbstractButton.IconOnly
                Accessible.name: text
                onClicked: rowMenu.popup()

                QQC2.Menu {
                    id: rowMenu

                    QQC2.MenuItem {
                        text: i18n("Rename…")
                        icon.name: "edit-rename"
                        onTriggered: {
                            renameField.text = row.title
                            renameDialog.chatId = row.chatId
                            renameDialog.open()
                        }
                    }

                    QQC2.MenuItem {
                        text: i18n("Delete")
                        icon.name: "edit-delete"
                        onTriggered: {
                            deleteDialog.chatId = row.chatId
                            deleteDialog.open()
                        }
                    }
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 2
            visible: chatList.count === 0
            icon.name: "mail-message"
            text: searchField.text.length > 0 ? i18n("No chats match your search") : i18n("No chats yet")
        }
    }

    Kirigami.PromptDialog {
        id: renameDialog

        property var chatId: 0

        title: i18n("Rename chat")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: page.controller.renameChat(chatId, renameField.text)

        QQC2.TextField {
            id: renameField

            Layout.fillWidth: true
            Accessible.name: i18n("Chat title")
            onAccepted: renameDialog.accept()
        }
    }

    Kirigami.PromptDialog {
        id: deleteDialog

        property var chatId: 0

        title: i18n("Delete chat")
        subtitle: i18n("This chat and all of its messages will be permanently deleted.")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: page.controller.deleteChat(chatId)
    }
}
