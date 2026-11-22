// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import io.github.elcrostatina.kompanion

Rectangle {
    id: block

    required property string code
    required property string language

    readonly property bool darkBackground: color.hslLightness < 0.5
    readonly property int copiedFeedbackMs: 1500
    readonly property real darkenFactor: 1.1

    function copyCode() {
        codeEdit.selectAll()
        codeEdit.copy()
        codeEdit.deselect()
        copiedTimer.restart()
    }

    implicitHeight: layout.implicitHeight
    radius: Kirigami.Units.cornerRadius
    color: Qt.darker(Kirigami.Theme.backgroundColor, darkenFactor)
    border.width: 1
    border.color: Qt.alpha(Kirigami.Theme.textColor, 0.15)

    CodeBlockHighlighter {
        textDocument: codeEdit.textDocument
        language: block.language
        darkBackground: block.darkBackground
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing * 2

            QQC2.Label {
                Layout.fillWidth: true
                text: block.language
                elide: Text.ElideRight
                opacity: 0.7
                font: Kirigami.Theme.smallFont
            }

            QQC2.ToolButton {
                icon.name: copiedTimer.running ? "dialog-ok" : "edit-copy"
                text: copiedTimer.running ? i18n("Copied") : i18n("Copy")
                display: QQC2.AbstractButton.TextBesideIcon
                onClicked: block.copyCode()
            }
        }

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: codeEdit.implicitHeight + Kirigami.Units.smallSpacing * 2
            contentWidth: codeEdit.implicitWidth + Kirigami.Units.smallSpacing * 4
            contentHeight: height
            clip: true
            flickableDirection: Flickable.HorizontalFlick
            boundsBehavior: Flickable.StopAtBounds

            TextEdit {
                id: codeEdit

                x: Kirigami.Units.smallSpacing * 2
                y: Kirigami.Units.smallSpacing
                text: block.code
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.NoWrap
                textFormat: TextEdit.PlainText
                font: Kirigami.Theme.fixedWidthFont
                color: Kirigami.Theme.textColor
                selectedTextColor: Kirigami.Theme.highlightedTextColor
                selectionColor: Kirigami.Theme.highlightColor
            }
        }
    }

    Timer {
        id: copiedTimer
        interval: block.copiedFeedbackMs
    }
}
