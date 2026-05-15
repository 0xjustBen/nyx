import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Effects
import Nyx

Rectangle {
    color: Theme.bg

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.parent.availableWidth
            anchors.margins: Theme.paddingLg
            spacing: Theme.gapXl

            // Header.
            ColumnLayout {
                Layout.topMargin: Theme.paddingLg
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                spacing: 4
                Label {
                    text: "PRESENCE"
                    color: Theme.textMute
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 3
                }
                Label {
                    text: "Appear as " + App.mode
                    color: Theme.text
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontHero
                    font.weight: Font.DemiBold
                }
                Label {
                    text: "Your Riot client sees the real chat. Friends see what you choose."
                    color: Theme.textDim
                    font.pixelSize: Theme.fontMd
                }
            }

            // Mode grid.
            GridLayout {
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                Layout.fillWidth: true
                columns: 5
                columnSpacing: Theme.gap
                rowSpacing: Theme.gap

                ModePill { mode: "online";  label: "Online";    glyph: "●"; dot: Theme.online }
                ModePill { mode: "away";    label: "Away";      glyph: "◐"; dot: Theme.away }
                ModePill { mode: "mobile";  label: "Mobile";    glyph: "▮"; dot: Theme.mobile }
                ModePill { mode: "dnd";     label: "Do Not Disturb"; glyph: "◯"; dot: Theme.dnd }
                ModePill { mode: "offline"; label: "Invisible"; glyph: "○"; dot: Theme.offline }
            }

            // Action row.
            RowLayout {
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                Layout.topMargin: Theme.gapLg
                spacing: Theme.gap

                // Primary CTA.
                Button {
                    text: "Launch Riot"
                    implicitHeight: 44
                    onClicked: App.launchRiot()
                    contentItem: RowLayout {
                        spacing: Theme.gapSm
                        Label {
                            text: "▶"
                            color: Theme.bg
                            font.pixelSize: Theme.fontSm
                        }
                        Label {
                            text: parent.parent.text
                            color: Theme.bg
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontMd
                            font.weight: Font.DemiBold
                        }
                    }
                    background: Rectangle {
                        radius: Theme.radius
                        color: parent.hovered ? Theme.accentHi : Theme.accent
                        Behavior on color { ColorAnimation { duration: Theme.durFast } }
                    }
                }

                Button {
                    text: "Install certificate"
                    flat: true
                    implicitHeight: 44
                    onClicked: App.installCert()
                    contentItem: Label {
                        text: parent.text
                        color: Theme.text
                        font.pixelSize: Theme.fontMd
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        radius: Theme.radius
                        color: "transparent"
                        border.color: parent.hovered ? Theme.borderHi : Theme.border
                        border.width: 1
                    }
                }

                Button {
                    text: "Uninstall certificate"
                    flat: true
                    implicitHeight: 44
                    onClicked: App.uninstallCert()
                    contentItem: Label {
                        text: parent.text
                        color: Theme.textDim
                        font.pixelSize: Theme.fontMd
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle { color: "transparent" }
                }

                Item { Layout.fillWidth: true }
            }

            // Telemetry.
            RowLayout {
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                Layout.topMargin: Theme.gapXl
                spacing: Theme.gap

                Repeater {
                    model: [
                        { label: "PROXY",     val: App.connected ? "up" : "idle" },
                        { label: "MODE",      val: App.mode },
                        { label: "FRIENDS",   val: App.roster.rowCount() },
                        { label: "REGION",    val: "na1" },
                    ]
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 72
                        radius: Theme.radius
                        color: Theme.surface
                        border.color: Theme.border
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: Theme.padding
                            spacing: 4
                            Label {
                                text: modelData.label
                                color: Theme.textMute
                                font.family: Theme.fontMono
                                font.pixelSize: Theme.fontXs
                                font.letterSpacing: 2
                            }
                            Label {
                                text: modelData.val
                                color: Theme.text
                                font.family: Theme.fontDisplay
                                font.pixelSize: Theme.fontXl
                                font.weight: Font.Medium
                            }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: Theme.paddingLg }
        }
    }
}
