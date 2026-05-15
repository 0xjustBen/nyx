import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Button {
    id: pill
    property string mode
    property string label
    property string glyph: "●"
    property color dot

    flat: true
    checkable: true
    checked: App.mode === mode
    onClicked: App.mode = mode

    Layout.fillWidth: true
    implicitHeight: 76

    background: Rectangle {
        radius: Theme.radius
        color: pill.checked ? Theme.surface2
             : pill.hovered ? Theme.surface
                            : Qt.rgba(1,1,1,0.015)
        border.color: pill.checked ? Theme.accent : Theme.border
        border.width: pill.checked ? 1.5 : 1
        Behavior on color       { ColorAnimation { duration: Theme.durFast } }
        Behavior on border.color { ColorAnimation { duration: Theme.durFast } }
    }

    contentItem: ColumnLayout {
        spacing: 6
        RowLayout {
            spacing: 6
            Layout.leftMargin: 14
            Layout.topMargin: 12
            Rectangle {
                width: 12; height: 12; radius: 6
                color: pill.dot
                Rectangle {
                    visible: pill.checked
                    anchors.centerIn: parent
                    width: 22; height: 22; radius: 11
                    color: "transparent"
                    border.color: pill.dot
                    border.width: 1
                    opacity: 0.6
                    SequentialAnimation on opacity {
                        running: pill.checked
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.2; duration: 1100 }
                        NumberAnimation { to: 0.6; duration: 1100 }
                    }
                }
            }
            Label {
                text: pill.glyph
                color: Theme.textDim
                font.pixelSize: Theme.fontXs
                font.family: Theme.fontMono
            }
            Item { Layout.fillWidth: true }
        }
        Label {
            text: pill.label
            color: pill.checked ? Theme.text : Theme.textDim
            font.family: Theme.fontDisplay
            font.pixelSize: Theme.fontMd
            font.weight: pill.checked ? Font.DemiBold : Font.Normal
            Layout.leftMargin: 14
            Layout.bottomMargin: 12
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
