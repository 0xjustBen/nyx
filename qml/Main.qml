import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Effects
import Qt.labs.platform as Platform
import Nyx

ApplicationWindow {
    id: root
    width: 1040
    height: 680
    minimumWidth: 880
    minimumHeight: 560
    visible: true
    title: "Nyx"
    color: Theme.bg
    flags: Qt.platform.os === "osx"
           ? Qt.Window
           : (Qt.Window | Qt.FramelessWindowHint)

    property string currentScreen: "status"
    property bool logOpen: false

    Platform.SystemTrayIcon {
        visible: true
        icon.source: "qrc:/qt/qml/Nyx/resources/icons/tray.svg"
        tooltip: "Nyx — " + App.mode
        onActivated: root.raise()
        menu: Platform.Menu {
            Platform.MenuItem { text: "Online";    onTriggered: App.mode = "online" }
            Platform.MenuItem { text: "Away";      onTriggered: App.mode = "away" }
            Platform.MenuItem { text: "Mobile";    onTriggered: App.mode = "mobile" }
            Platform.MenuItem { text: "Invisible"; onTriggered: App.mode = "offline" }
            Platform.MenuItem { separator: true }
            Platform.MenuItem { text: "Quit"; onTriggered: App.quit() }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar.
        Rectangle {
            Layout.preferredWidth: 232
            Layout.fillHeight: true
            color: Theme.bgAlt

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.padding
                spacing: Theme.gapLg

                // Brand mark.
                RowLayout {
                    spacing: Theme.gap
                    Rectangle {
                        width: 28; height: 28; radius: 8
                        gradient: Gradient {
                            orientation: Gradient.Vertical
                            GradientStop { position: 0.0; color: Theme.accent }
                            GradientStop { position: 1.0; color: Theme.online }
                        }
                        Label {
                            anchors.centerIn: parent
                            text: "N"
                            color: Theme.bg
                            font.family: Theme.fontDisplay
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }
                    ColumnLayout {
                        spacing: 0
                        Label {
                            text: "NYX"
                            color: Theme.text
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontLg
                            font.letterSpacing: 4
                            font.bold: true
                        }
                        Label {
                            text: "v0.1 · alpha"
                            color: Theme.textMute
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontXs
                        }
                    }
                }

                Item { Layout.preferredHeight: Theme.gapSm }

                // Status pill.
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    radius: Theme.radius
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Theme.padding
                        anchors.rightMargin: Theme.padding
                        spacing: Theme.gap

                        Rectangle {
                            width: 10; height: 10; radius: 5
                            color: App.connected ? Theme.online : Theme.offline
                            SequentialAnimation on opacity {
                                running: App.connected
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.4; duration: 900; easing.type: Easing.InOutSine }
                                NumberAnimation { to: 1.0; duration: 900; easing.type: Easing.InOutSine }
                            }
                        }
                        ColumnLayout {
                            spacing: 0
                            Layout.fillWidth: true
                            Label {
                                text: App.connected ? "ACTIVE" : "STANDBY"
                                color: Theme.text
                                font.family: Theme.fontMono
                                font.pixelSize: Theme.fontXs
                                font.letterSpacing: 2
                            }
                            Label {
                                text: App.status
                                color: Theme.textDim
                                font.pixelSize: Theme.fontSm
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }
                    }
                }

                // Nav.
                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Repeater {
                        model: [
                            { key: "status",   label: "Presence" },
                            { key: "roster",   label: "Friends"  },
                            { key: "settings", label: "Settings" },
                        ]
                        delegate: ItemDelegate {
                            Layout.fillWidth: true
                            implicitHeight: 40
                            text: modelData.label
                            highlighted: root.currentScreen === modelData.key
                            onClicked: root.currentScreen = modelData.key

                            contentItem: RowLayout {
                                spacing: Theme.gap
                                Rectangle {
                                    width: 3; height: 18; radius: 1.5
                                    color: parent.parent.highlighted ? Theme.accent : "transparent"
                                    Behavior on color { ColorAnimation { duration: Theme.durFast } }
                                }
                                Label {
                                    text: parent.parent.text
                                    color: parent.parent.highlighted ? Theme.text : Theme.textDim
                                    font.family: Theme.fontDisplay
                                    font.pixelSize: Theme.fontMd
                                    Layout.fillWidth: true
                                }
                            }
                            background: Rectangle {
                                radius: Theme.radiusSm
                                color: parent.highlighted ? Theme.surface
                                     : parent.hovered     ? Qt.rgba(1,1,1,0.03)
                                                          : "transparent"
                                Behavior on color { ColorAnimation { duration: Theme.durFast } }
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Log toggle.
                Button {
                    Layout.fillWidth: true
                    flat: true
                    onClicked: root.logOpen = !root.logOpen
                    contentItem: RowLayout {
                        Label {
                            text: root.logOpen ? "▾ Hide log" : "▴ Show log"
                            color: Theme.textDim
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 1
                            Layout.fillWidth: true
                        }
                    }
                    background: Rectangle {
                        radius: Theme.radiusSm
                        color: parent.hovered ? Theme.surface : "transparent"
                    }
                }
            }
        }

        // Main pane + log drawer.
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            StackLayout {
                id: stack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.currentScreen === "status" ? 0
                            : root.currentScreen === "roster" ? 1
                                                              : 2

                StatusScreen   {}
                RosterScreen   {}
                SettingsScreen {}
            }

            // Log drawer.
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: root.logOpen ? 180 : 0
                clip: true
                color: Theme.bgAlt
                Behavior on Layout.preferredHeight { NumberAnimation { duration: Theme.durMed; easing.type: Easing.OutCubic } }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: Theme.border
                }

                ScrollView {
                    anchors.fill: parent
                    anchors.topMargin: 1
                    anchors.margins: Theme.padding
                    TextArea {
                        id: logArea
                        readOnly: true
                        color: Theme.textDim
                        background: null
                        font.family: Theme.fontMono
                        font.pixelSize: Theme.fontXs
                        wrapMode: TextArea.Wrap
                    }
                }
            }
        }
    }

    Connections {
        target: App
        function onLogLine(line) {
            const ts = Qt.formatTime(new Date(), "HH:mm:ss")
            logArea.append("<font color='" + Theme.textMute + "'>" + ts + "</font>  " + line)
        }
    }
}
