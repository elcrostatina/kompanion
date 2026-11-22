// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: delegate

    required property string author
    required property string blockKind
    required property string blockText
    required property string language
    required property bool firstInMessage

    readonly property bool isUser: author === "user"
    readonly property bool isCode: blockKind === "code"
    readonly property int bubblePadding: Kirigami.Units.smallSpacing * 2

    function authorLabel() {
        if (author === "user") {
            return i18n("You")
        }
        return author === "system" ? i18n("System") : i18n("Assistant")
    }

    width: ListView.view ? ListView.view.width : implicitWidth
    implicitHeight: column.implicitHeight + (firstInMessage ? Kirigami.Units.largeSpacing : 0)

    ColumnLayout {
        id: column

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true
            visible: delegate.firstInMessage
            text: delegate.authorLabel()
            font.bold: true
            color: delegate.isUser ? Kirigami.Theme.textColor : Kirigami.Theme.highlightColor
        }

        CodeBlock {
            Layout.fillWidth: true
            visible: delegate.isCode
            code: delegate.isCode ? delegate.blockText : ""
            language: delegate.language
        }

        Rectangle {
            Layout.fillWidth: true
            visible: !delegate.isCode
            implicitHeight: textEdit.implicitHeight + (delegate.isUser ? delegate.bubblePadding * 2 : 0)
            radius: Kirigami.Units.cornerRadius
            color: delegate.isUser ? Kirigami.Theme.alternateBackgroundColor : "transparent"

            TextEdit {
                id: textEdit

                x: delegate.isUser ? delegate.bubblePadding : 0
                y: delegate.isUser ? delegate.bubblePadding : 0
                width: parent.width - (delegate.isUser ? delegate.bubblePadding * 2 : 0)
                text: delegate.isCode ? "" : delegate.blockText
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.MarkdownText
                font: Kirigami.Theme.defaultFont
                color: Kirigami.Theme.textColor
                selectedTextColor: Kirigami.Theme.highlightedTextColor
                selectionColor: Kirigami.Theme.highlightColor
                onLinkActivated: link => Qt.openUrlExternally(link)
            }
        }
    }
}
