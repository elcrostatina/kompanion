// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    id: card

    required property var editor
    required property int index
    required property int count
    required property string providerId
    required property string name
    required property string kind
    required property string iconName
    required property var url
    required property string userAgent
    required property bool providerEnabled
    required property bool overridesEnabled

    property bool expanded: false
    property bool apiKeyStored: false

    readonly property bool isWeb: kind === "web"

    signal removeRequested(string providerId, string name)

    Component.onCompleted: if (!isWeb) editor.checkApiKey(providerId)

    Connections {
        target: card.editor

        function onApiKeyChecked(checkedId, present) {
            if (checkedId === card.providerId) {
                card.apiKeyStored = present
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            QQC2.Switch {
                checked: card.providerEnabled
                Accessible.name: i18n("Show %1 in the panel", card.name)
                onToggled: card.editor.setEnabled(card.providerId, checked)
            }

            QQC2.Label {
                Layout.fillWidth: true
                text: card.name
                elide: Text.ElideRight
                font.bold: true
            }

            QQC2.ToolButton {
                icon.name: card.expanded ? "arrow-up" : "arrow-down"
                text: card.expanded ? i18n("Hide details") : i18n("Show details")
                display: QQC2.AbstractButton.IconOnly
                onClicked: card.expanded = !card.expanded
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: text
            }
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            visible: card.expanded
            wideMode: false

            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                Kirigami.FormData.label: i18n("Order:")

                QQC2.Button {
                    text: i18n("Move up")
                    icon.name: "go-up"
                    enabled: card.index > 0
                    onClicked: card.editor.moveUp(card.providerId)
                }

                QQC2.Button {
                    text: i18n("Move down")
                    icon.name: "go-down"
                    enabled: card.index < card.count - 1
                    onClicked: card.editor.moveDown(card.providerId)
                }
            }

            QQC2.TextField {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Name:")
                text: card.name
                onEditingFinished: card.editor.setName(card.providerId, text)
            }

            QQC2.TextField {
                Layout.fillWidth: true
                Kirigami.FormData.label: card.isWeb ? i18n("Address:") : i18n("API address:")
                text: card.url.toString()
                placeholderText: card.isWeb ? "https://" : "http://localhost:1234/v1"
                inputMethodHints: Qt.ImhUrlCharactersOnly
                onEditingFinished: card.editor.setUrl(card.providerId, text)
            }

            QQC2.TextField {
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Icon:")
                text: card.iconName
                placeholderText: i18n("Icon name or file; empty for a letter")
                onEditingFinished: card.editor.setIconName(card.providerId, text)
            }

            QQC2.TextField {
                Layout.fillWidth: true
                visible: card.isWeb
                Kirigami.FormData.label: i18n("User agent:")
                text: card.userAgent
                placeholderText: i18n("Default (current Chrome)")
                onEditingFinished: card.editor.setUserAgent(card.providerId, text)
            }

            SwitchRow {
                visible: card.isWeb
                Kirigami.FormData.label: i18n("Style overrides:")
                checked: card.overridesEnabled
                label: checked ? i18n("On") : i18n("Off")
                onToggled: card.editor.setOverridesEnabled(card.providerId, checked)
            }

            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                visible: card.isWeb
                Kirigami.FormData.label: i18n("Override files:")

                QQC2.Button {
                    text: i18n("Edit CSS")
                    icon.name: "document-edit"
                    onClicked: card.editor.editCss(card.providerId)
                }

                QQC2.Button {
                    text: i18n("Edit JS")
                    icon.name: "document-edit"
                    onClicked: card.editor.editJavaScript(card.providerId)
                }

                QQC2.Button {
                    text: i18n("Reset")
                    icon.name: "document-revert"
                    onClicked: card.editor.resetCss(card.providerId)
                }
            }

            Kirigami.PasswordField {
                Layout.fillWidth: true
                id: apiKeyField

                visible: !card.isWeb
                Kirigami.FormData.label: i18n("API key:")
                placeholderText: card.apiKeyStored ? i18n("Stored in your wallet") : i18n("Optional")
                onAccepted: saveKeyButton.clicked()
            }

            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                visible: !card.isWeb

                QQC2.Button {
                    id: saveKeyButton

                    text: i18n("Save key")
                    icon.name: "document-save"
                    enabled: apiKeyField.text.length > 0
                    onClicked: {
                        card.editor.storeApiKey(card.providerId, apiKeyField.text)
                        apiKeyField.clear()
                    }
                }

                QQC2.Button {
                    text: i18n("Remove key")
                    icon.name: "edit-delete"
                    enabled: card.apiKeyStored
                    onClicked: card.editor.removeApiKey(card.providerId)
                }
            }

            QQC2.Button {
                visible: card.isWeb
                text: i18n("Clear site data")
                icon.name: "edit-clear-history"
                onClicked: card.editor.clearSiteData(card.providerId)
            }

            QQC2.Button {
                text: i18n("Remove provider")
                icon.name: "edit-delete"
                onClicked: card.removeRequested(card.providerId, card.name)
            }
        }
    }
}
