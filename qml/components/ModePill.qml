import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Button {
    id: pill
    property string mode
    property string label
    property string kbd: ""

    readonly property color tone: Theme.modeColor(mode)
    readonly property bool active: App.mode === mode

    flat: true
    Layout.fillWidth: true
    Layout.maximumWidth: 110
    implicitHeight: 90
    Layout.alignment: Qt.AlignTop
    onClicked: App.mode = mode

    background: Rectangle {
        radius: Theme.r
        color: pill.active ? Qt.rgba(pill.tone.r, pill.tone.g, pill.tone.b, 0.08)
             : pill.hovered ? Theme.surface2 : Theme.surface
        border.color: pill.active ? pill.tone : (pill.hovered ? Theme.line2 : Theme.line)
        border.width: 1
        Behavior on color        { ColorAnimation { duration: Theme.durFast } }
        Behavior on border.color { ColorAnimation { duration: Theme.durFast } }

        // Top-edge accent.
        Rectangle {
            visible: pill.active
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: -1
            width: 24
            height: 2
            color: pill.tone
        }
    }

    contentItem: ColumnLayout {
        spacing: 6
        anchors.fill: parent
        Item { Layout.preferredHeight: 4 }

        // Icon-circle.
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 28; height: 28; radius: 14
            color: "transparent"
            border.color: pill.tone
            border.width: 1
            Label {
                anchors.centerIn: parent
                text: Theme.modeGlyph(pill.mode)
                color: pill.tone
                font.pixelSize: 11
            }
        }

        Label {
            text: pill.label.toUpperCase()
            color: Theme.fg
            font.family: Theme.fontDisplay
            font.pixelSize: Theme.fontXs
            font.weight: Font.DemiBold
            font.letterSpacing: 1
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: pill.kbd
            color: Theme.muted
            font.family: Theme.fontMono
            font.pixelSize: 9
            font.letterSpacing: 1
            Layout.alignment: Qt.AlignHCenter
            visible: pill.kbd.length > 0
        }

        Item { Layout.fillHeight: true }
    }
}
