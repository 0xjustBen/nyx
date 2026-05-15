import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

ApplicationWindow {
    id: root
    width: 980
    height: 640
    minimumWidth: 760
    minimumHeight: 480
    visible: true
    title: "Nyx"
    color: Theme.bg
    flags: Qt.Window | Qt.FramelessWindowHint

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TitleBar {
            Layout.fillWidth: true
            onCloseClicked: App.quit()
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 220
                Layout.fillHeight: true
                color: Theme.surface
                border.color: Theme.border
                border.width: 0

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.padding
                    spacing: Theme.gap

                    Label {
                        text: "NYX"
                        color: Theme.accent
                        font.pixelSize: 22
                        font.bold: true
                        font.letterSpacing: 4
                    }

                    Label {
                        text: App.connected ? "connected" : "idle"
                        color: App.connected ? Theme.online : Theme.textDim
                        font.pixelSize: 12
                    }

                    Item { Layout.fillHeight: true }

                    Repeater {
                        model: [
                            { key: "status",   label: "Status" },
                            { key: "roster",   label: "Friends" },
                            { key: "settings", label: "Settings" },
                        ]
                        delegate: ItemDelegate {
                            Layout.fillWidth: true
                            text: modelData.label
                            highlighted: stack.currentKey === modelData.key
                            onClicked: stack.currentKey = modelData.key
                            contentItem: Label {
                                text: parent.text
                                color: parent.highlighted ? Theme.text : Theme.textDim
                                font.pixelSize: 14
                            }
                            background: Rectangle {
                                color: parent.highlighted ? Theme.surface2 : "transparent"
                                radius: Theme.radiusSm
                            }
                        }
                    }
                }
            }

            StackLayout {
                id: stack
                Layout.fillWidth: true
                Layout.fillHeight: true
                property string currentKey: "status"
                currentIndex: currentKey === "status" ? 0
                            : currentKey === "roster" ? 1
                            : 2

                StatusScreen   {}
                RosterScreen   {}
                SettingsScreen {}
            }
        }
    }
}
