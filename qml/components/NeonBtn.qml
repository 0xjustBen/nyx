import QtQuick
import QtQuick.Controls.Basic
import Nyx

Button {
    id: btn
    property bool primary: false
    property bool danger: false
    flat: true

    implicitHeight: 34
    leftPadding: 14; rightPadding: 14

    background: Rectangle {
        radius: Theme.rSm
        color: btn.danger  ? (btn.hovered ? Qt.lighter(Theme.red, 1.1) : Theme.red)
             : btn.primary ? (btn.hovered ? Qt.lighter(Theme.cyan, 1.05) : Theme.cyan)
             : (btn.hovered ? Theme.surface2 : Theme.surface)
        border.color: btn.danger ? Theme.red
                    : btn.primary ? Theme.cyan
                    : (btn.hovered ? Theme.cyan : Theme.line2)
        border.width: 1
        Behavior on color        { ColorAnimation { duration: Theme.durFast } }
        Behavior on border.color { ColorAnimation { duration: Theme.durFast } }
    }

    contentItem: Label {
        text: btn.text
        color: (btn.primary || btn.danger) ? Theme.bgDeep
             : (btn.hovered ? Theme.cyan : Theme.fg)
        font.family: Theme.fontDisplay
        font.pixelSize: Theme.fontXs
        font.letterSpacing: 1.5
        font.weight: Font.DemiBold
        font.capitalization: Font.AllUppercase
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
