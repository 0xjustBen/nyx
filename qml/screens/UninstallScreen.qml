import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    Rectangle {
        anchors.centerIn: parent
        width: 540
        radius: Theme.rLg
        color: Theme.surface
        border.color: Theme.line2
        border.width: 1
        implicitHeight: stackCol.implicitHeight + 72

        // Red accent line.
        Rectangle {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: -1
            width: parent.width - 48
            height: 2
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.5; color: Theme.red }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        ColumnLayout {
            id: stackCol
            anchors.fill: parent
            anchors.margins: 36
            spacing: 20

            Rectangle {
                width: 54; height: 54; radius: 27
                color: "transparent"
                border.color: Theme.red
                border.width: 1
                Layout.alignment: Qt.AlignLeft
                Label {
                    anchors.centerIn: parent
                    text: "!"
                    color: Theme.red
                    font.family: Theme.fontDisplay
                    font.pixelSize: 22
                    font.weight: Font.Bold
                }
            }

            Label {
                text: "Remove Nyx from this machine"
                color: Theme.fg
                font.family: Theme.fontDisplay
                font.pixelSize: 22
                font.weight: Font.Bold
                font.letterSpacing: -0.3
            }
            Label {
                text: "Nyx will revert your Riot Client to vanilla behavior. Your Riot account, friends, and game data are untouched — they live with Riot, not with Nyx."
                color: Theme.muted2
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontSm
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0
                Repeater {
                    model: [
                        { ck: true,  text: "Remove Nyx Root CA from user trust store", tag: "cert" },
                        { ck: true,  text: "Tear down local proxy on 127.0.0.1:5223",  tag: "proxy" },
                        { ck: true,  text: "Disable autostart at login",                tag: "launchd" },
                        { ck: true,  text: "Delete cached friend roster & activity log", tag: "data" },
                        { ck: false, text: "Keep preferences (region, defaults) for reinstall", tag: "opt-in" },
                    ]
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        color: "transparent"
                        Rectangle {
                            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                            height: 1
                            color: index < 4 ? Theme.line : "transparent"
                        }
                        RowLayout {
                            anchors.fill: parent
                            spacing: 10
                            Label {
                                text: modelData.ck ? "✓" : "○"
                                color: modelData.ck ? Theme.cyan : Theme.muted
                                font.pixelSize: Theme.fontMd
                                Layout.preferredWidth: 18
                            }
                            Label {
                                text: modelData.text
                                color: modelData.ck ? Theme.fg : Theme.muted
                                font.family: Theme.fontMono
                                font.pixelSize: Theme.fontBase
                                Layout.fillWidth: true
                            }
                            Label {
                                text: modelData.tag.toUpperCase()
                                color: Theme.muted
                                font.family: Theme.fontDisplay
                                font.pixelSize: 9
                                font.letterSpacing: 2
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 8
                spacing: 10
                NeonBtn { text: "Cancel" }
                Item { Layout.fillWidth: true }
                NeonBtn { text: "Quit only" }
                NeonBtn { text: "Remove Nyx"; danger: true; onClicked: { App.uninstallCert(); App.quit() } }
            }
        }
    }
}
