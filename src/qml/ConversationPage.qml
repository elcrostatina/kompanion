// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: page

    required property var controller

    property bool stickToBottom: true

    signal chatListRequested()

    function titleOrPlaceholder() {
        return controller.currentTitle.length > 0 ? controller.currentTitle : i18n("New chat")
    }

    spacing: 0

    QQC2.ToolBar {
        Layout.fillWidth: true

        RowLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            QQC2.ToolButton {
                icon.name: "view-list-details"
                text: i18n("Chats")
                display: QQC2.AbstractButton.IconOnly
                Accessible.name: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                onClicked: page.chatListRequested()
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: page.titleOrPlaceholder()
                elide: Text.ElideRight
                font.bold: true
            }

            QQC2.ToolButton {
                icon.name: "list-add"
                text: i18n("New chat")
                display: QQC2.AbstractButton.IconOnly
                enabled: !page.controller.busy
                Accessible.name: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                onClicked: page.controller.newChat()
            }

            QQC2.ToolButton {
                icon.name: "overflow-menu"
                text: i18n("More")
                display: QQC2.AbstractButton.IconOnly
                Accessible.name: text
                onClicked: actionsMenu.popup()

                QQC2.Menu {
                    id: actionsMenu

                    QQC2.MenuItem {
                        text: i18n("System prompt…")
                        icon.name: "configure"
                        onTriggered: {
                            systemPromptField.text = page.controller.systemPrompt
                            systemPromptDialog.open()
                        }
                    }

                    QQC2.MenuItem {
                        text: i18n("Export as Markdown…")
                        icon.name: "document-export"
                        enabled: page.controller.currentChatId !== 0
                        onTriggered: exportDialog.start("markdown")
                    }

                    QQC2.MenuItem {
                        text: i18n("Export as JSON…")
                        icon.name: "document-export"
                        enabled: page.controller.currentChatId !== 0
                        onTriggered: exportDialog.start("json")
                    }

                    QQC2.MenuItem {
                        text: i18n("Import chat from JSON…")
                        icon.name: "document-import"
                        onTriggered: importDialog.open()
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing

        QQC2.ComboBox {
            Layout.fillWidth: true
            model: page.controller.models
            enabled: !page.controller.busy && count > 0
            displayText: page.controller.currentModel.length > 0 ? page.controller.currentModel : i18n("No model")
            currentIndex: model.indexOf(page.controller.currentModel)
            Accessible.name: i18n("Model")
            onActivated: index => page.controller.selectModel(model[index])
        }

        QQC2.BusyIndicator {
            running: page.controller.loadingModels
            visible: running
            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
        }

        QQC2.ToolButton {
            icon.name: "view-refresh"
            text: i18n("Refresh models")
            display: QQC2.AbstractButton.IconOnly
            enabled: !page.controller.loadingModels
            Accessible.name: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: text
            onClicked: page.controller.refreshModels()
        }
    }

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.smallSpacing
        type: Kirigami.MessageType.Error
        text: page.controller.errorText
        visible: page.controller.errorText.length > 0
        showCloseButton: true
        onVisibleChanged: if (!visible) page.controller.dismissError()
    }

    ListView {
        id: messageList

        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        model: page.controller.messages
        boundsBehavior: Flickable.StopAtBounds
        spacing: Kirigami.Units.smallSpacing

        delegate: MessageDelegate {}

        QQC2.ScrollBar.vertical: QQC2.ScrollBar {}

        onMovementEnded: page.stickToBottom = atYEnd
        onContentHeightChanged: if (page.stickToBottom) positionViewAtEnd()

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 2
            visible: messageList.count === 0
            icon.name: "computer"
            text: i18n("Chat with %1", page.controller.providerName)
            explanation: page.controller.models.length === 0 ? i18n("Waiting for a model. Make sure the server is running and a model is loaded.") : i18n("Type a message below to start.")
        }
    }

    Composer {
        id: composer

        Layout.fillWidth: true
        busy: page.controller.busy
        onSendRequested: text => {
            page.stickToBottom = true
            page.controller.sendMessage(text)
        }
        onStopRequested: page.controller.stop()
    }

    Component.onCompleted: composer.focusInput()

    Kirigami.PromptDialog {
        id: systemPromptDialog

        title: i18n("System prompt")
        subtitle: i18n("Instructions the model receives before every message in this chat.")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: page.controller.changeSystemPrompt(systemPromptField.text)

        QQC2.TextArea {
            id: systemPromptField

            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 8
            wrapMode: TextEdit.Wrap
            Accessible.name: i18n("System prompt")
        }
    }

    FileDialog {
        id: exportDialog

        property string format: "markdown"

        function start(chosenFormat) {
            format = chosenFormat
            defaultSuffix = chosenFormat === "markdown" ? "md" : "json"
            open()
        }

        fileMode: FileDialog.SaveFile
        onAccepted: page.controller.exportChat(page.controller.currentChatId, format, selectedFile)
    }

    FileDialog {
        id: importDialog

        fileMode: FileDialog.OpenFile
        nameFilters: [i18n("Chat files (*.json)")]
        onAccepted: page.controller.importChat(selectedFile)
    }
}
