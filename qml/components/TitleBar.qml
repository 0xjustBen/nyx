import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    id: bar
    height: 40
    color: Theme.surface
    signal closeClicked()

    MouseArea {
        anchors.fill: parent
        property point startPos
        onPressed: function(e) { startPos = Qt.point(e.x, e.y) }
        onPositionChanged: function(e) {
            if (pressed) {
                const w = bar.Window.window
                w.x += e.x - startPos.x
                w.y += e.y - startPos.y
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.padding
        anchors.rightMargin: 4
        spacing: Theme.gap

        Label {
            text: "Nyx"
            color: Theme.textDim
            font.pixelSize: 12
            font.letterSpacing: 2
        }

        Item { Layout.fillWidth: true }

        Button {
            text: "—"
            flat: true
            onClicked: bar.Window.window.showMinimized()
            contentItem: Label { text: parent.text; color: Theme.textDim; horizontalAlignment: Text.AlignHCenter }
            background: Rectangle { color: parent.hovered ? Theme.surface2 : "transparent" }
            implicitWidth: 40
        }

        Button {
            text: "×"
            flat: true
            onClicked: bar.closeClicked()
            contentItem: Label { text: parent.text; color: Theme.textDim; horizontalAlignment: Text.AlignHCenter; font.pixelSize: 16 }
            background: Rectangle { color: parent.hovered ? Theme.danger : "transparent" }
            implicitWidth: 40
        }
    }
}
