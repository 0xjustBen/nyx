import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.gapLg
        spacing: Theme.gapLg

        Label {
            text: "Presence"
            color: Theme.text
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: "Riot client sees you as " + App.mode + "."
            color: Theme.textDim
            font.pixelSize: 13
        }

        GridLayout {
            columns: 3
            columnSpacing: Theme.gap
            rowSpacing: Theme.gap

            ModePill { mode: "online";    label: "Online";    dot: Theme.online }
            ModePill { mode: "away";      label: "Away";      dot: Theme.away }
            ModePill { mode: "mobile";    label: "Mobile";    dot: Theme.online }
            ModePill { mode: "invisible"; label: "Invisible"; dot: Theme.offline }
            ModePill { mode: "offline";   label: "Offline";   dot: Theme.offline }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.border
        }

        RowLayout {
            spacing: Theme.gap
            Button {
                text: "Install certificate"
                onClicked: App.installCert()
            }
            Button {
                text: "Patch client"
                onClicked: App.patchClient()
            }
            Button {
                text: "Restore"
                flat: true
                onClicked: { App.restoreClient(); App.uninstallCert() }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
