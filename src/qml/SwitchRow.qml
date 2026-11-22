// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

RowLayout {
    id: row

    property alias checked: toggle.checked
    property alias label: caption.text

    signal toggled()

    Layout.fillWidth: true
    spacing: Kirigami.Units.smallSpacing

    QQC2.Switch {
        id: toggle

        Accessible.name: caption.text
        onToggled: row.toggled()
    }

    QQC2.Label {
        id: caption

        Layout.fillWidth: true
        wrapMode: Text.Wrap
    }
}
