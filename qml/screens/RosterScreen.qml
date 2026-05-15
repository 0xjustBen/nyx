import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.paddingLg
        spacing: Theme.gapLg

        ColumnLayout {
            spacing: 4
            Label {
                text: "FRIENDS"
                color: Theme.textMute
                font.family: Theme.fontMono
                font.pixelSize: Theme.fontXs
                font.letterSpacing: 3
            }
            Label {
                text: "Roster"
                color: Theme.text
                font.family: Theme.fontDisplay
                font.pixelSize: Theme.fontXxl
                font.weight: Font.DemiBold
            }
            Label {
                text: App.roster.rowCount() + " contacts · live from XMPP"
                color: Theme.textDim
                font.pixelSize: Theme.fontSm
            }
        }

        TextField {
            id: search
            Layout.fillWidth: true
            placeholderText: "Search friends"
            color: Theme.text
            placeholderTextColor: Theme.textMute
            font.pixelSize: Theme.fontMd
            leftPadding: 36
            background: Rectangle {
                radius: Theme.radius
                color: Theme.surface
                border.color: search.activeFocus ? Theme.accent : Theme.border
                border.width: 1
                Behavior on border.color { ColorAnimation { duration: Theme.durFast } }
            }
            Label {
                text: "⌕"
                anchors.left: parent.left
                anchors.leftMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.textMute
                font.pixelSize: Theme.fontLg
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Theme.radius
            color: Theme.surface
            border.color: Theme.border
            border.width: 1
            clip: true

            ListView {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 0
                model: App.roster
                delegate: FriendRow {
                    width: ListView.view.width
                    friendName: model.name
                    friendPresence: model.presence
                    friendGame: model.game
                    visible: search.text.length === 0 ||
                             model.name.toLowerCase().indexOf(search.text.toLowerCase()) >= 0
                }

                Label {
                    anchors.centerIn: parent
                    visible: App.roster.rowCount() === 0
                    text: "No friends yet — launch Riot to populate."
                    color: Theme.textMute
                    font.pixelSize: Theme.fontSm
                }
            }
        }
    }
}
