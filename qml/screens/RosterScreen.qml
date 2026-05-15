import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapLg
        spacing: Theme.gap

        Label {
            text: "Friends"
            color: Theme.text
            font.pixelSize: 24
            font.bold: true
        }

        TextField {
            id: search
            Layout.fillWidth: true
            placeholderText: "Search"
            color: Theme.text
            placeholderTextColor: Theme.textDim
            background: Rectangle {
                radius: Theme.radiusSm
                color: Theme.surface
                border.color: search.activeFocus ? Theme.accent : Theme.border
                border.width: 1
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: App.roster
            delegate: FriendRow {
                friendName: model.name
                friendPresence: model.presence
                friendGame: model.game
                visible: search.text.length === 0 ||
                         model.name.toLowerCase().indexOf(search.text.toLowerCase()) >= 0
            }
        }
    }
}
