import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

ItemDelegate {
    id: row
    property string friendName
    property string friendPresence
    property string friendGame

    height: 64

    background: Rectangle {
        color: row.hovered ? Qt.rgba(1,1,1,0.03) : "transparent"
        radius: Theme.radiusSm
        Behavior on color { ColorAnimation { duration: Theme.durFast } }
    }

    contentItem: RowLayout {
        spacing: Theme.gap
        anchors.fill: parent
        anchors.leftMargin: Theme.padding
        anchors.rightMargin: Theme.padding

        // Avatar with presence ring.
        Item {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            Rectangle {
                anchors.fill: parent
                radius: width / 2
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: Theme.accent }
                    GradientStop { position: 1.0; color: Theme.accentDim }
                }
                Label {
                    anchors.centerIn: parent
                    text: row.friendName.length ? row.friendName.charAt(0).toUpperCase() : "?"
                    color: Theme.text
                    font.family: Theme.fontDisplay
                    font.bold: true
                    font.pixelSize: Theme.fontMd
                }
            }
            Rectangle {
                width: 12; height: 12; radius: 6
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                border.color: Theme.surface
                border.width: 2
                color: Theme.presenceColor(row.friendPresence)
            }
        }

        ColumnLayout {
            spacing: 2
            Layout.fillWidth: true
            Label {
                text: row.friendName
                color: Theme.text
                font.family: Theme.fontDisplay
                font.pixelSize: Theme.fontMd
                font.weight: Font.Medium
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            RowLayout {
                spacing: 6
                Label {
                    text: row.friendPresence || "offline"
                    color: Theme.presenceColor(row.friendPresence)
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                }
                Label {
                    visible: row.friendGame.length > 0
                    text: "·"
                    color: Theme.textMute
                    font.pixelSize: Theme.fontXs
                }
                Label {
                    visible: row.friendGame.length > 0
                    text: row.friendGame
                    color: Theme.textDim
                    font.pixelSize: Theme.fontXs
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
        }
    }
}
