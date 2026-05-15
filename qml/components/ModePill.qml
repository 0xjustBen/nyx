import QtQuick
import QtQuick.Controls.Basic
import Nyx

Button {
    id: pill
    property string mode
    property string label
    property color dot

    flat: true
    checkable: true
    checked: App.mode === mode
    onClicked: App.mode = mode

    implicitHeight: 44
    implicitWidth: 140

    background: Rectangle {
        radius: Theme.radius
        color: pill.checked ? Theme.surface2 : "transparent"
        border.color: pill.checked ? Theme.accent : Theme.border
        border.width: 1
        Behavior on color { ColorAnimation { duration: 120 } }
    }

    contentItem: Row {
        spacing: 10
        leftPadding: 14
        Rectangle {
            width: 10; height: 10; radius: 5
            color: pill.dot
            anchors.verticalCenter: parent.verticalCenter
        }
        Label {
            text: pill.label
            color: Theme.text
            font.pixelSize: 14
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
