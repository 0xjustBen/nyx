import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Qt.labs.platform as Platform
import Nyx

ApplicationWindow {
    id: root
    width: 1180
    height: 760
    minimumWidth: 980
    minimumHeight: 640
    visible: true
    title: "Nyx — Presence Control"
    color: Theme.bgDeep
    flags: Qt.platform.os === "osx" ? Qt.Window : (Qt.Window | Qt.FramelessWindowHint)

    property string currentScreen: "main"  // main | settings | activity | uninstall

    // Keyboard shortcuts for mode switching.
    Shortcut { sequences: ["Ctrl+1", "Meta+1"]; onActivated: App.mode = "online" }
    Shortcut { sequences: ["Ctrl+2", "Meta+2"]; onActivated: App.mode = "away" }
    Shortcut { sequences: ["Ctrl+3", "Meta+3"]; onActivated: App.mode = "mobile" }
    Shortcut { sequences: ["Ctrl+4", "Meta+4"]; onActivated: App.mode = "dnd" }
    Shortcut { sequences: ["Ctrl+5", "Meta+5"]; onActivated: App.mode = "offline" }
    Shortcut { sequences: ["Ctrl+L", "Meta+L"]; onActivated: root.currentScreen = "activity" }
    Shortcut { sequences: ["Ctrl+,", "Meta+,"]; onActivated: root.currentScreen = "settings" }
    Shortcut { sequences: ["Ctrl+Return", "Meta+Return"]; onActivated: App.launchRiot() }

    Platform.SystemTrayIcon {
        id: tray
        visible: true
        icon.source: "qrc:/qt/qml/Nyx/resources/icons/tray.svg"
        tooltip: "Nyx · " + App.mode
        onActivated: root.raise()
        menu: Platform.Menu {
            Platform.MenuItem { text: "Online";    onTriggered: App.mode = "online" }
            Platform.MenuItem { text: "Away";      onTriggered: App.mode = "away" }
            Platform.MenuItem { text: "Mobile";    onTriggered: App.mode = "mobile" }
            Platform.MenuItem { text: "DND";       onTriggered: App.mode = "dnd" }
            Platform.MenuItem { text: "Invisible"; onTriggered: App.mode = "offline" }
            Platform.MenuItem { separator: true }
            Platform.MenuItem { text: App.connected ? "Pause Nyx" : "Resume Nyx"
                                onTriggered: App.pause(App.connected) }
            Platform.MenuItem { text: "Launch Riot"; onTriggered: App.launchRiot() }
            Platform.MenuItem { separator: true }
            Platform.MenuItem { text: "Quit"; onTriggered: App.quit() }
        }
    }

    Connections {
        target: App
        function onModeChanged() {
            tray.showMessage("Nyx", "Friends now see you as " + App.mode,
                             Platform.SystemTrayIcon.Information, 1500)
        }
    }

    // Background with radial glows.
    Rectangle {
        anchors.fill: parent
        color: Theme.bgDeep
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: Qt.rgba(0.337, 0.800, 0.878, 0.05) }
                GradientStop { position: 0.6; color: "transparent" }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Titlebar (frameless on Win/Linux).
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 38
            color: Theme.surface
            visible: Qt.platform.os !== "osx"

            Rectangle {
                anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                height: 1; color: Theme.line
            }

            MouseArea {
                anchors.fill: parent
                property point startPos
                onPressed: function(e) { startPos = Qt.point(e.x, e.y) }
                onPositionChanged: function(e) {
                    if (pressed) {
                        const w = root
                        w.x += e.x - startPos.x
                        w.y += e.y - startPos.y
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 4
                spacing: 10

                Row {
                    spacing: 7
                    Rectangle { width: 11; height: 11; radius: 5.5; color: "#E26956" }
                    Rectangle { width: 11; height: 11; radius: 5.5; color: "#E8B45D" }
                    Rectangle { width: 11; height: 11; radius: 5.5; color: "#56D096" }
                }

                Label {
                    text: "NYX · "
                    color: Theme.muted2
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontSm
                    font.letterSpacing: 3
                    font.bold: true
                }
                Label {
                    text: root.currentScreen === "main"     ? "PRESENCE CONTROL"
                         : root.currentScreen === "settings" ? "SETTINGS"
                         : root.currentScreen === "activity" ? "ACTIVITY"
                         : "UNINSTALL"
                    color: Theme.cyan
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontSm
                    font.letterSpacing: 3
                    font.bold: true
                }
                Item { Layout.fillWidth: true }

                Repeater {
                    model: [
                        { key: "main",     label: "Main" },
                        { key: "activity", label: "Activity" },
                        { key: "settings", label: "Settings" },
                    ]
                    delegate: Label {
                        text: modelData.label
                        color: root.currentScreen === modelData.key ? Theme.cyan : Theme.muted
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontXs
                        font.letterSpacing: 2
                        Layout.rightMargin: 8
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentScreen = modelData.key
                        }
                    }
                }

                // Connection pill.
                Rectangle {
                    radius: 99
                    color: "transparent"
                    border.color: Theme.line2
                    border.width: 1
                    implicitHeight: 22
                    implicitWidth: pillRow.implicitWidth + 18
                    RowLayout {
                        id: pillRow
                        anchors.centerIn: parent
                        spacing: 6
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: App.connected ? Theme.cyan : Theme.grey
                            SequentialAnimation on opacity {
                                running: App.connected
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.4; duration: 1000 }
                                NumberAnimation { to: 1.0; duration: 1000 }
                            }
                        }
                        Label {
                            text: (App.connected ? "CONNECTED" : "STANDBY") + " · EUW1"
                            color: Theme.cyan
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 1.5
                        }
                    }
                }

                Button {
                    text: "—"; flat: true; implicitWidth: 38
                    onClicked: root.showMinimized()
                    contentItem: Label { text: parent.text; color: Theme.muted; horizontalAlignment: Text.AlignHCenter }
                    background: Rectangle { color: parent.hovered ? Theme.surface2 : "transparent" }
                }
                Button {
                    text: "×"; flat: true; implicitWidth: 38
                    onClicked: App.quit()
                    contentItem: Label { text: parent.text; color: Theme.muted; horizontalAlignment: Text.AlignHCenter; font.pixelSize: 16 }
                    background: Rectangle { color: parent.hovered ? Theme.red : "transparent" }
                }
            }
        }

        // Top nav strip on macOS (where no frameless titlebar).
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 38
            color: Theme.surface
            visible: Qt.platform.os === "osx"

            Rectangle {
                anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                height: 1; color: Theme.line
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 16

                Label {
                    text: "NYX · "
                    color: Theme.muted2
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontSm
                    font.letterSpacing: 3
                    font.bold: true
                }
                Label {
                    text: root.currentScreen === "main"     ? "PRESENCE CONTROL"
                         : root.currentScreen === "settings" ? "SETTINGS"
                         : root.currentScreen === "activity" ? "ACTIVITY"
                         : "UNINSTALL"
                    color: Theme.cyan
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontSm
                    font.letterSpacing: 3
                    font.bold: true
                }
                Item { Layout.fillWidth: true }

                Rectangle {
                    radius: 99
                    color: "transparent"
                    border.color: Theme.line2
                    border.width: 1
                    implicitHeight: 22
                    implicitWidth: pillRow2.implicitWidth + 18
                    RowLayout {
                        id: pillRow2
                        anchors.centerIn: parent
                        spacing: 6
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: App.connected ? Theme.cyan : Theme.grey
                            SequentialAnimation on opacity {
                                running: App.connected
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.4; duration: 1000 }
                                NumberAnimation { to: 1.0; duration: 1000 }
                            }
                        }
                        Label {
                            text: (App.connected ? "CONNECTED" : "STANDBY") + " · EUW1"
                            color: Theme.cyan
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 1.5
                        }
                    }
                }

                Repeater {
                    model: [
                        { key: "main",     label: "Main" },
                        { key: "activity", label: "Activity" },
                        { key: "settings", label: "Settings" },
                    ]
                    delegate: Label {
                        text: modelData.label
                        color: root.currentScreen === modelData.key ? Theme.cyan : Theme.muted
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontXs
                        font.letterSpacing: 2
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.currentScreen = modelData.key
                        }
                    }
                }
            }
        }

        // Body switcher.
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentScreen === "main"     ? 0
                        : root.currentScreen === "activity" ? 1
                        : root.currentScreen === "settings" ? 2
                                                            : 3

            MainScreen     {}
            ActivityScreen {}
            SettingsScreen {}
            UninstallScreen{}
        }

        // Status bar.
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: Theme.bgDeep

            Rectangle {
                anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                height: 1; color: Theme.line
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 18

                RowLayout {
                    spacing: 6
                    Rectangle {
                        width: 5; height: 5; radius: 2.5
                        color: Theme.cyan
                        SequentialAnimation on opacity {
                            running: App.connected
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 1100 }
                            NumberAnimation { to: 1.0; duration: 1100 }
                        }
                    }
                    Label {
                        text: App.connected ? "NYX ACTIVE" : "STANDBY"
                        color: Theme.cyan
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontXs
                        font.letterSpacing: 1.5
                    }
                }
                Label {
                    text: "RIOT CHAT · XMPP LINKED"
                    color: Theme.muted
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 1.5
                }
                Label {
                    text: "CERT VALID · 89 DAYS"
                    color: Theme.muted
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 1.5
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: App.status
                    color: Theme.muted
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                    elide: Text.ElideRight
                }
                Label {
                    text: "v 0.1.0-alpha"
                    color: Theme.muted
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 1.5
                }
            }
        }
    }
}
