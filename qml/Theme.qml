pragma Singleton
import QtQuick

QtObject {
    readonly property color bg:        "#0B0C10"
    readonly property color surface:   "#15171C"
    readonly property color surface2:  "#1E2129"
    readonly property color border:    "#272B33"
    readonly property color text:      "#E8EAED"
    readonly property color textDim:   "#9AA1AC"
    readonly property color accent:    "#7C5CFF"
    readonly property color accentDim: "#4A38B0"
    readonly property color online:    "#3FBF7F"
    readonly property color away:      "#E0B040"
    readonly property color offline:   "#6E7480"
    readonly property color danger:    "#E2495C"

    readonly property int radius: 12
    readonly property int radiusSm: 8
    readonly property int gap: 12
    readonly property int gapLg: 20
    readonly property int padding: 16

    readonly property string fontFamily: "Inter, -apple-system, Segoe UI, sans-serif"
}
