// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page

    required property var panel
    required property var preferences
    required property var providerEditor
    required property var allProviders

    property string pendingRemovalId: ""
    property string pendingRemovalName: ""

    title: i18n("Settings")

    header: QQC2.ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            QQC2.ToolButton {
                icon.name: "go-previous"
                text: i18n("Back")
                display: QQC2.AbstractButton.IconOnly
                Accessible.name: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
                onClicked: page.QQC2.StackView.view.pop()
            }

            Kirigami.Heading {
                Layout.fillWidth: true
                level: 3
                text: page.title
                elide: Text.ElideRight
            }
        }
    }

    Connections {
        target: page.providerEditor

        function onSiteDataCleared(providerId) {
            clearedNotice.visible = true
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            id: clearedNotice

            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            showCloseButton: true
            visible: false
            text: i18n("Cookies and cache were cleared. Remaining site data is removed the next time Kompanion starts.")
        }

        Kirigami.Heading {
            level: 3
            text: i18n("General")
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            wideMode: false

            RowLayout {
                Kirigami.FormData.label: i18n("Open on:")

                QQC2.RadioButton {
                    text: i18n("Left")
                    checked: page.panel.side === "left"
                    onClicked: page.panel.side = "left"
                }

                QQC2.RadioButton {
                    text: i18n("Right")
                    checked: page.panel.side === "right"
                    onClicked: page.panel.side = "right"
                }
            }

            SwitchRow {
                Kirigami.FormData.label: i18n("Style:")
                label: i18n("Floating, with rounded corners")
                checked: page.panel.floating
                onToggled: page.panel.floating = checked
            }

            SwitchRow {
                Kirigami.FormData.label: i18n("Autostart:")
                label: i18n("Start Kompanion when I log in")
                checked: page.preferences.autostartEnabled
                onToggled: page.preferences.autostartEnabled = checked
            }

            SwitchRow {
                Kirigami.FormData.label: i18n("Web pages:")
                label: i18n("Match the Plasma dark/light color scheme")
                checked: page.preferences.followPlasmaColorScheme
                onToggled: page.preferences.followPlasmaColorScheme = checked
            }

            QQC2.Label {
                Layout.fillWidth: true
                visible: page.preferences.restartRequired
                text: i18n("Restart Kompanion to apply this change.")
                wrapMode: Text.Wrap
                color: Kirigami.Theme.neutralTextColor
            }

            QQC2.Button {
                Kirigami.FormData.label: i18n("Shortcut:")
                text: i18n("Change shortcut…")
                icon.name: "preferences-desktop-keyboard-shortcut"
                onClicked: page.preferences.openShortcutSettings()
            }

            QQC2.TextArea {
                Layout.fillWidth: true
                Layout.maximumWidth: Number.POSITIVE_INFINITY
                Layout.preferredHeight: Kirigami.Units.gridUnit * 5
                Kirigami.FormData.label: i18n("Default system prompt:")
                Kirigami.FormData.labelAlignment: Qt.AlignTop
                placeholderText: i18n("Used for new chats with local providers")
                text: page.preferences.defaultSystemPrompt
                wrapMode: TextEdit.Wrap
                onTextChanged: if (activeFocus) page.preferences.defaultSystemPrompt = text
                Accessible.name: i18n("Default system prompt")
            }
        }

        Kirigami.Heading {
            level: 3
            text: i18n("Providers")
        }

        Repeater {
            id: providerRepeater

            model: page.allProviders

            delegate: ProviderSettingsCard {
                editor: page.providerEditor
                count: providerRepeater.count
                onRemoveRequested: (providerId, name) => {
                    page.pendingRemovalId = providerId
                    page.pendingRemovalName = name
                    removeDialog.open()
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            QQC2.Button {
                text: i18n("Add web provider")
                icon.name: "list-add"
                onClicked: addDialog.openFor("web")
            }

            QQC2.Button {
                text: i18n("Add local provider")
                icon.name: "list-add"
                onClicked: addDialog.openFor("local")
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: i18n("Override files live in ~/.config/kompanion/overrides. If signing in fails inside the panel, use “Open sign-in page in your browser” in the page toolbar.")
            wrapMode: Text.Wrap
            opacity: 0.7
            font: Kirigami.Theme.smallFont
        }
    }

    AddProviderDialog {
        id: addDialog

        editor: page.providerEditor
    }

    Kirigami.PromptDialog {
        id: removeDialog

        title: i18n("Remove provider")
        subtitle: i18n("Remove %1? Its stored login data and API key are deleted.", page.pendingRemovalName)
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: page.providerEditor.remove(page.pendingRemovalId)
    }
}
