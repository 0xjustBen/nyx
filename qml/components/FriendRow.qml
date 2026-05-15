import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

ItemDelegate {
    id: row
    property string friendName
    property string friendTag: ""
    property string friendActivity: ""
    property string friendGame: ""
    property string friendPresence: "offline"

    height: 58

    readonly property color presence: Theme.presenceColor(friendPresence)
    readonly property color gameColor:
          friendGame === "LOL"  ? Theme.gameLol
        : friendGame === "VAL"  ? Theme.gameVal
        : friendGame === "LOR"  ? Theme.gameLor
        : friendGame === "2XKO" ? Theme.game2xko
                                : Theme.muted

    background: Rectangle {
        color: row.hovered ? Theme.surface : "transparent"
        Rectangle {
            anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
            width: 2
            color: row.hovered ? Theme.line2 : "transparent"
        }
    }

    contentItem: RowLayout {
        spacing: 12
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        // Avatar with presence dot.
        Item {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: "transparent"
                border.color: Theme.line
                border.width: 1
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: Theme.surface2 }
                    GradientStop { position: 1.0; color: Theme.line2 }
                }
                Label {
                    anchors.centerIn: parent
                    text: row.friendName.length >= 2
                          ? (row.friendName.charAt(0) + row.friendName.charAt(1)).toUpperCase()
                          : (row.friendName.charAt(0) || "?").toUpperCase()
                    color: Theme.muted2
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontMd
                    font.weight: Font.DemiBold
                }
            }
            Rectangle {
                width: 11; height: 11; radius: 5.5
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: -2
                anchors.bottomMargin: -2
                color: row.presence
                border.color: Theme.bg
                border.width: 2
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            RowLayout {
                spacing: 6
                Label {
                    text: row.friendName
                    color: Theme.fg
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontBase
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Label {
                    text: row.friendTag
                    color: Theme.muted
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontSm
                }
            }
            Label {
                text: row.friendActivity
                color: Theme.muted
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontXs
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        Rectangle {
            visible: row.friendGame.length > 0
            color: "transparent"
            border.color: row.gameColor
            border.width: 1
            radius: 3
            implicitWidth: gameLbl.implicitWidth + 12
            implicitHeight: 18
            Label {
                id: gameLbl
                anchors.centerIn: parent
                text: row.friendGame
                color: row.gameColor
                font.family: Theme.fontDisplay
                font.pixelSize: 9
                font.weight: Font.DemiBold
                font.letterSpacing: 2
            }
        }
    }
}
