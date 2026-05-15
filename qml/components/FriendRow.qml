import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

ItemDelegate {
    id: row
    property string friendName
    property string friendPresence
    property string friendGame

    height: 56
    width: parent ? parent.width : 320

    background: Rectangle {
        color: row.hovered ? Theme.surface2 : "transparent"
        radius: Theme.radiusSm
    }

    contentItem: RowLayout {
        spacing: Theme.gap
        anchors.fill: parent
        anchors.leftMargin: Theme.padding
        anchors.rightMargin: Theme.padding

        Rectangle {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            radius: 18
            color: Theme.accentDim
            Label {
                anchors.centerIn: parent
                text: row.friendName.length ? row.friendName.charAt(0).toUpperCase() : "?"
                color: Theme.text
                font.bold: true
            }
            Rectangle {
                width: 12; height: 12; radius: 6
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                border.color: Theme.surface
                border.width: 2
                color: row.friendPresence === "chat"     ? Theme.online
                     : row.friendPresence === "away"     ? Theme.away
                     : row.friendPresence === "dnd"      ? Theme.danger
                     : Theme.offline
            }
        }

        ColumnLayout {
            spacing: 2
            Layout.fillWidth: true
            Label {
                text: row.friendName
                color: Theme.text
                font.pixelSize: 14
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Label {
                text: row.friendGame.length ? row.friendGame : row.friendPresence
                color: Theme.textDim
                font.pixelSize: 12
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
    }
}
