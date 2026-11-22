// SPDX-FileCopyrightText: 2026 Kompanion contributors
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts
import QtWebEngine
import org.kde.kirigami as Kirigami

Item {
    id: root

    required property var webProfile

    property bool loadFailed: false
    property color pageColor: Kirigami.Theme.backgroundColor

    readonly property color pageTextColor: Kirigami.ColorUtils.brightnessForColor(pageColor) === Kirigami.ColorUtils.Light ? "#232629" : "#fcfcfc"
    readonly property real chromeBorderAlpha: 0.12
    readonly property int colorResampleDelayMs: 1500
    readonly property int colorWatchIntervalMs: 4000
    readonly property bool windowShown: Window.window ? Window.window.visible : false

    readonly property real minimumZoom: 0.25
    readonly property real maximumZoom: 5.0
    readonly property real zoomStep: 0.1
    readonly property real defaultZoom: 1.0
    readonly property int progressBarHeight: 2
    readonly property int noticeDurationMs: 6000
    readonly property int percentScale: 100
    readonly property int abortedErrorCode: -3

    function setZoom(factor) {
        const clamped = Math.max(minimumZoom, Math.min(maximumZoom, factor))
        webView.zoomFactor = Math.round(clamped * percentScale) / percentScale
        webProfile.rememberZoomFactor(webView.zoomFactor)
    }

    function zoomBy(delta) {
        setZoom(webView.zoomFactor + delta)
    }

    function isMicrophoneOrCamera(permission) {
        const type = permission.permissionType
        return type === WebEnginePermission.PermissionType.MediaAudioCapture
            || type === WebEnginePermission.PermissionType.MediaVideoCapture
            || type === WebEnginePermission.PermissionType.MediaAudioVideoCapture
    }

    function describeCapture(permission) {
        const type = permission.permissionType
        if (type === WebEnginePermission.PermissionType.MediaAudioCapture) {
            return i18n("use your microphone")
        }
        if (type === WebEnginePermission.PermissionType.MediaVideoCapture) {
            return i18n("use your camera")
        }
        return i18n("use your microphone and camera")
    }

    function handlePermission(permission) {
        if (permission.permissionType === WebEnginePermission.PermissionType.ClipboardReadWrite) {
            permission.grant()
        } else if (isMicrophoneOrCamera(permission)) {
            capturePrompt.ask(permission)
        } else {
            permission.deny()
        }
    }

    function samplePageColor() {
        webView.runJavaScript(webProfile.pageColorScript, result => {
            if (typeof result === "string" && result.length > 0) {
                pageColor = result
            }
        })
    }

    function openExternally(address) {
        Qt.openUrlExternally(address)
    }

    Shortcut {
        sequences: [StandardKey.ZoomIn, "Ctrl+="]
        enabled: root.visible
        onActivated: root.zoomBy(root.zoomStep)
    }

    Shortcut {
        sequence: StandardKey.ZoomOut
        enabled: root.visible
        onActivated: root.zoomBy(-root.zoomStep)
    }

    Shortcut {
        sequence: "Ctrl+0"
        enabled: root.visible
        onActivated: root.setZoom(root.defaultZoom)
    }

    onWindowShownChanged: if (windowShown) colorResampleTimer.restart()

    Timer {
        id: colorWatchTimer

        interval: root.colorWatchIntervalMs
        repeat: true
        running: root.windowShown && root.visible
        onTriggered: root.samplePageColor()
    }

    Timer {
        id: colorResampleTimer

        interval: root.colorResampleDelayMs
        onTriggered: root.samplePageColor()
    }

    Rectangle {
        anchors.fill: parent
        color: root.pageColor

        Behavior on color {
            ColorAnimation {
                duration: Kirigami.Units.longDuration
            }
        }
    }

    Connections {
        target: root.webProfile

        function onScriptsChanged() {
            webView.runJavaScript(root.webProfile.styleScript)
        }

        function onDownloadSaved(filePath) {
            downloadNotice.text = i18n("Saved to %1", filePath)
            downloadNotice.visible = true
            downloadNoticeTimer.restart()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: parent.width * webView.loadProgress / root.percentScale
            Layout.preferredHeight: root.progressBarHeight
            visible: webView.loading
            color: Kirigami.Theme.highlightColor
        }

        Kirigami.InlineMessage {
            id: downloadNotice

            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            showCloseButton: true
            visible: false

            Timer {
                id: downloadNoticeTimer
                interval: root.noticeDurationMs
                onTriggered: downloadNotice.visible = false
            }
        }

        WebEngineView {
            id: webView

            Layout.fillWidth: true
            Layout.fillHeight: true
            profile: root.webProfile.profile
            backgroundColor: root.pageColor
            zoomFactor: root.webProfile.zoomFactor
            settings.javascriptCanAccessClipboard: true
            settings.javascriptCanPaste: true

            userScripts.collection: [
                {
                    name: "kompanion-style-override",
                    injectionPoint: WebEngineScript.DocumentReady,
                    worldId: WebEngineScript.MainWorld,
                    sourceCode: root.webProfile.styleScript
                },
                {
                    name: "kompanion-user-script",
                    injectionPoint: WebEngineScript.DocumentReady,
                    worldId: WebEngineScript.MainWorld,
                    sourceCode: root.webProfile.userScript
                }
            ]

            Component.onCompleted: url = root.webProfile.startUrl

            onNavigationRequested: request => {
                const isUserClick = request.navigationType === WebEngineNavigationRequest.LinkClickedNavigation
                if (isUserClick && request.isMainFrame && !root.webProfile.staysInsidePanel(request.url)) {
                    request.action = WebEngineNavigationRequest.IgnoreRequest
                    root.openExternally(request.url)
                }
            }

            onNewWindowRequested: request => {
                if (root.webProfile.staysInsidePanel(request.requestedUrl)) {
                    webView.url = request.requestedUrl
                } else {
                    root.openExternally(request.requestedUrl)
                }
            }

            onLoadingChanged: loadingInfo => {
                if (loadingInfo.status === WebEngineView.LoadStartedStatus) {
                    root.loadFailed = false
                } else if (loadingInfo.status === WebEngineView.LoadSucceededStatus) {
                    root.samplePageColor()
                    colorResampleTimer.restart()
                } else if (loadingInfo.status === WebEngineView.LoadFailedStatus) {
                    const isHttpStatus = loadingInfo.errorDomain === WebEngineLoadingInfo.HttpStatusCodeDomain
                    root.loadFailed = !isHttpStatus && loadingInfo.errorCode !== root.abortedErrorCode
                }
            }

            onPermissionRequested: permission => root.handlePermission(permission)

            onFileDialogRequested: request => {
                request.accepted = true
                fileChooser.start(request)
            }

            onContextMenuRequested: request => {
                request.accepted = true
                contextMenu.show(request)
            }
        }

        QQC2.ToolBar {
            Layout.fillWidth: true
            position: QQC2.ToolBar.Footer

            Kirigami.Theme.inherit: false
            Kirigami.Theme.backgroundColor: root.pageColor
            Kirigami.Theme.textColor: root.pageTextColor

            background: Rectangle {
                color: root.pageColor

                Rectangle {
                    width: parent.width
                    height: 1
                    color: Qt.alpha(root.pageTextColor, root.chromeBorderAlpha)
                }
            }

            RowLayout {
                anchors.fill: parent
                spacing: 0

                QQC2.ToolButton {
                    icon.name: "go-previous"
                    text: i18n("Back")
                    display: QQC2.AbstractButton.IconOnly
                    enabled: webView.canGoBack
                    onClicked: webView.goBack()
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                }

                QQC2.ToolButton {
                    icon.name: webView.loading ? "process-stop" : "view-refresh"
                    text: webView.loading ? i18n("Stop") : i18n("Reload")
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: webView.loading ? webView.stop() : webView.reload()
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                }

                Item {
                    Layout.fillWidth: true
                }

                QQC2.ToolButton {
                    icon.name: "zoom-out"
                    text: i18n("Zoom out")
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.zoomBy(-root.zoomStep)
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                }

                QQC2.ToolButton {
                    text: Math.round(webView.zoomFactor * root.percentScale) + "%"
                    onClicked: root.setZoom(root.defaultZoom)
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18n("Reset zoom")
                }

                QQC2.ToolButton {
                    icon.name: "zoom-in"
                    text: i18n("Zoom in")
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.zoomBy(root.zoomStep)
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                }

                Item {
                    Layout.fillWidth: true
                }

                QQC2.ToolButton {
                    icon.name: "internet-web-browser"
                    text: i18n("Open this page in your browser")
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.openExternally(webView.url)
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                }

                QQC2.ToolButton {
                    icon.name: "user-identity"
                    text: i18n("Open sign-in page in your browser")
                    display: QQC2.AbstractButton.IconOnly
                    onClicked: root.openExternally(root.webProfile.startUrl)
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: text
                }
            }
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: parent.width - Kirigami.Units.gridUnit * 2
        visible: root.loadFailed
        icon.name: "network-disconnect"
        text: i18n("Could not load %1", root.webProfile.providerName)
        helpfulAction: Kirigami.Action {
            icon.name: "view-refresh"
            text: i18n("Try again")
            onTriggered: webView.reload()
        }
    }

    Kirigami.PromptDialog {
        id: capturePrompt

        property var pendingPermission: null

        function ask(permission) {
            if (pendingPermission) {
                pendingPermission.deny()
            }
            pendingPermission = permission
            subtitle = i18n("%1 wants to %2.", root.webProfile.providerName, root.describeCapture(permission))
            open()
        }

        function resolve(allowed) {
            if (pendingPermission) {
                allowed ? pendingPermission.grant() : pendingPermission.deny()
                pendingPermission = null
            }
            close()
        }

        title: i18n("Permission request")
        standardButtons: Kirigami.Dialog.NoButton
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Allow")
                icon.name: "dialog-ok"
                onTriggered: capturePrompt.resolve(true)
            },
            Kirigami.Action {
                text: i18n("Deny")
                icon.name: "dialog-cancel"
                onTriggered: capturePrompt.resolve(false)
            }
        ]
        onClosed: if (pendingPermission) resolve(false)
    }

    FileDialog {
        id: fileChooser

        property var pendingRequest: null

        function start(request) {
            pendingRequest = request
            fileMode = request.mode === FileDialogRequest.FileModeOpenMultiple ? FileDialog.OpenFiles : FileDialog.OpenFile
            open()
        }

        onAccepted: {
            pendingRequest.dialogAccept(selectedFiles)
            pendingRequest = null
        }
        onRejected: {
            pendingRequest.dialogReject()
            pendingRequest = null
        }
    }

    QQC2.Menu {
        id: contextMenu

        property var currentRequest: null

        function show(request) {
            currentRequest = request
            popup(webView, request.position.x, request.position.y)
        }

        readonly property bool hasLink: currentRequest !== null && currentRequest.linkUrl.toString().length > 0
        readonly property bool isEditable: currentRequest !== null && currentRequest.isContentEditable
        readonly property bool hasSelection: currentRequest !== null && currentRequest.selectedText.length > 0

        QQC2.MenuItem {
            text: i18n("Cut")
            icon.name: "edit-cut"
            enabled: contextMenu.isEditable && contextMenu.hasSelection
            onTriggered: webView.triggerWebAction(WebEngineView.Cut)
        }

        QQC2.MenuItem {
            text: i18n("Copy")
            icon.name: "edit-copy"
            enabled: contextMenu.hasSelection
            onTriggered: webView.triggerWebAction(WebEngineView.Copy)
        }

        QQC2.MenuItem {
            text: i18n("Paste")
            icon.name: "edit-paste"
            enabled: contextMenu.isEditable
            onTriggered: webView.triggerWebAction(WebEngineView.Paste)
        }

        QQC2.MenuItem {
            text: i18n("Select all")
            icon.name: "edit-select-all"
            onTriggered: webView.triggerWebAction(WebEngineView.SelectAll)
        }

        QQC2.MenuSeparator {}

        QQC2.MenuItem {
            text: i18n("Copy link address")
            icon.name: "edit-copy"
            enabled: contextMenu.hasLink
            onTriggered: webView.triggerWebAction(WebEngineView.CopyLinkToClipboard)
        }

        QQC2.MenuItem {
            text: i18n("Open link in your browser")
            icon.name: "internet-web-browser"
            enabled: contextMenu.hasLink
            onTriggered: root.openExternally(contextMenu.currentRequest.linkUrl)
        }
    }
}
