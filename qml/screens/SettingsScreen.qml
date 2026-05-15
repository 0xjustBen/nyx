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
            text: "Settings"
            color: Theme.text
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: "Region, autostart, certificate trust, advanced proxy options."
            color: Theme.textDim
            font.pixelSize: 13
        }

        Item { Layout.fillHeight: true }
    }
}
