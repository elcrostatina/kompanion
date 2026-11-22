// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Item {
    id: tile

    property string name: ""
    property string iconName: ""
    property string glyph: ""
    property bool selected: false
    property bool hovered: false
    property color tint: Qt.hsla(nameHue / hueCount, tintSaturation, tintLightness, 1)

    readonly property int size: Kirigami.Units.gridUnit * 2
    readonly property int hueCount: 360
    readonly property real tintSaturation: 0.65
    readonly property real tintLightness: 0.52
    readonly property real cornerFactor: 0.32
    readonly property real restingFillAlpha: 0.16
    readonly property real hoverFillAlpha: 0.28
    readonly property real borderAlpha: 0.4
    readonly property real darkThemeLetterBoost: 1.5
    readonly property real lightThemeLetterShade: 1.25
    readonly property real letterSizeFactor: 1.2
    readonly property bool darkTheme: Kirigami.ColorUtils.brightnessForColor(Kirigami.Theme.backgroundColor) === Kirigami.ColorUtils.Dark
    readonly property color letterColor: selected ? "white" : (darkTheme ? Qt.lighter(tint, darkThemeLetterBoost) : Qt.darker(tint, lightThemeLetterShade))
    readonly property string letter: glyph.length > 0 ? glyph : name.trim().charAt(0).toUpperCase()
    readonly property int nameHue: {
        let hash = 0
        for (let i = 0; i < name.length; ++i) {
            hash = (hash * 31 + name.charCodeAt(i)) % hueCount
        }
        return hash
    }

    implicitWidth: size
    implicitHeight: size

    Rectangle {
        anchors.fill: parent
        radius: tile.size * tile.cornerFactor
        color: tile.selected ? tile.tint : Qt.alpha(tile.tint, tile.hovered ? tile.hoverFillAlpha : tile.restingFillAlpha)
        border.width: tile.selected ? 0 : 1
        border.color: Qt.alpha(tile.tint, tile.borderAlpha)

        Behavior on color {
            ColorAnimation {
                duration: Kirigami.Units.shortDuration
            }
        }
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Kirigami.Units.iconSizes.smallMedium
        height: width
        visible: tile.iconName.length > 0
        source: tile.iconName
        color: tile.letterColor
        isMask: false
    }

    QQC2.Label {
        anchors.centerIn: parent
        visible: tile.iconName.length === 0
        text: tile.letter
        color: tile.letterColor
        font.bold: true
        font.pixelSize: Math.round(Kirigami.Theme.defaultFont.pixelSize * tile.letterSizeFactor)
    }
}
