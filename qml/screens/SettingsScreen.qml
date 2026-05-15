import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    property string section: "General"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar nav.
        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: Theme.bgDeep

            Rectangle {
                anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
                width: 1; color: Theme.line
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 24
                spacing: 0

                Repeater {
                    model: [
                        { kind: "grp",  text: "Preferences" },
                        { kind: "item", text: "General" },
                        { kind: "item", text: "Presence Rules" },
                        { kind: "item", text: "Friends & Roster" },
                        { kind: "item", text: "Notifications" },
                        { kind: "grp",  text: "System" },
                        { kind: "item", text: "Certificate" },
                        { kind: "item", text: "Network & Proxy" },
                        { kind: "item", text: "Hotkeys" },
                        { kind: "grp",  text: "Advanced" },
                        { kind: "item", text: "Activity Log" },
                        { kind: "item", text: "Diagnostics" },
                        { kind: "danger", text: "Uninstall" },
                    ]
                    delegate: Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: modelData.kind === "grp" ? 34 : 30
                        Layout.topMargin: modelData.kind === "grp" && index > 0 ? 14 : 0

                        Label {
                            visible: modelData.kind === "grp"
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.text.toUpperCase()
                            color: Theme.muted
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 3
                        }

                        Rectangle {
                            visible: modelData.kind !== "grp"
                            anchors.fill: parent
                            color: section === modelData.text ? Qt.rgba(0.337, 0.800, 0.878, 0.06)
                                : navMa.containsMouse ? Theme.surface : "transparent"
                            Rectangle {
                                anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
                                width: 2
                                color: section === modelData.text ? Theme.cyan : "transparent"
                            }
                            Label {
                                anchors.left: parent.left
                                anchors.leftMargin: 20
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.text
                                color: modelData.kind === "danger" ? Theme.red
                                     : section === modelData.text ? Theme.cyan
                                                                  : Theme.muted2
                                font.family: Theme.fontDisplay
                                font.pixelSize: Theme.fontBase
                            }
                            MouseArea {
                                id: navMa
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: section = modelData.text
                            }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
        }

        // Body.
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.parent.availableWidth
                anchors.margins: 44
                spacing: 0

                ColumnLayout {
                    spacing: 4
                    Layout.topMargin: 32
                    Layout.leftMargin: 44
                    Layout.rightMargin: 44
                    Layout.bottomMargin: 24
                    Label {
                        text: section
                        color: Theme.fg
                        font.family: Theme.fontDisplay
                        font.pixelSize: 24
                        font.weight: Font.Bold
                        font.letterSpacing: -0.3
                    }
                    Label {
                        text: "Region, defaults, and how Nyx behaves at launch."
                        color: Theme.muted
                        font.family: Theme.fontMono
                        font.pixelSize: Theme.fontBase
                    }
                }

                // Setting rows.
                Repeater {
                    model: [
                        { nm: "Region",                            ds: "Where Nyx connects to Riot Chat. Auto-detects from your account.", ctl: "select", val: "EUW · Western Europe" },
                        { nm: "Default startup mode",              ds: "Presence state to broadcast when Nyx launches.", ctl: "select", val: "Invisible" },
                        { nm: "Default game on launch",            ds: "Which Riot title the Launch button opens.", ctl: "select", val: "League of Legends" },
                        { nm: "Start Nyx with system",             ds: "Boot silently in the menu bar at login.", ctl: "toggle", on: true },
                        { nm: "Notify when friends come online",   ds: "Quiet notification — bypassed in DND mode.", ctl: "toggle", on: true },
                        { nm: "Notify on presence-spoof events",   ds: "Tell me whenever a friend's status mismatches Riot's reported value.", ctl: "toggle", on: false },
                        { nm: "Telemetry",                         ds: "Zero. Nyx is local-only by design. This switch does nothing — it's here to prove it.", ctl: "toggle-dead", on: false },
                    ]
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.leftMargin: 44
                        Layout.rightMargin: 44
                        Layout.preferredHeight: 70
                        color: "transparent"
                        Rectangle {
                            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                            height: 1; color: Theme.line
                        }
                        RowLayout {
                            anchors.fill: parent
                            spacing: 24
                            ColumnLayout {
                                Layout.preferredWidth: 280
                                spacing: 4
                                Label {
                                    text: modelData.nm
                                    color: Theme.fg
                                    font.family: Theme.fontDisplay
                                    font.pixelSize: Theme.fontMd
                                    font.weight: Font.DemiBold
                                }
                                Label {
                                    text: modelData.ds
                                    color: Theme.muted
                                    font.family: Theme.fontMono
                                    font.pixelSize: Theme.fontSm
                                    wrapMode: Text.WordWrap
                                    Layout.fillWidth: true
                                }
                            }
                            Item { Layout.fillWidth: true }

                            // Select control.
                            Rectangle {
                                visible: modelData.ctl === "select"
                                radius: Theme.rSm
                                color: Theme.surface
                                border.color: Theme.line2
                                border.width: 1
                                implicitHeight: 30
                                implicitWidth: selectLbl.implicitWidth + 44
                                Label {
                                    id: selectLbl
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.val || ""
                                    color: Theme.fg
                                    font.family: Theme.fontMono
                                    font.pixelSize: Theme.fontBase
                                }
                                Label {
                                    anchors.right: parent.right
                                    anchors.rightMargin: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: "▾"
                                    color: Theme.cyan
                                    font.pixelSize: Theme.fontXs
                                }
                            }

                            // Toggle.
                            Rectangle {
                                visible: modelData.ctl === "toggle" || modelData.ctl === "toggle-dead"
                                width: 38; height: 22; radius: 11
                                color: modelData.on ? Qt.rgba(0.337, 0.800, 0.878, 0.2) : Theme.surface2
                                border.color: modelData.on ? Theme.cyan : Theme.line2
                                border.width: 1
                                opacity: modelData.ctl === "toggle-dead" ? 0.4 : 1
                                Rectangle {
                                    width: 16; height: 16; radius: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    x: modelData.on ? parent.width - width - 2 : 2
                                    color: modelData.on ? Theme.cyan : Theme.muted
                                    Behavior on x { NumberAnimation { duration: Theme.durFast } }
                                }
                            }
                        }
                    }
                }

                Item { Layout.preferredHeight: 48 }
            }
        }
    }
}
