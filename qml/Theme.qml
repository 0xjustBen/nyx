pragma Singleton
import QtQuick

QtObject {
    // Base palette — OKLch-derived darks with cool neutrals.
    readonly property color bg:        "#070809"
    readonly property color bgAlt:     "#0C0E12"
    readonly property color surface:   "#12151B"
    readonly property color surface2:  "#1A1E26"
    readonly property color surface3:  "#222732"
    readonly property color border:    "#2A303D"
    readonly property color borderHi:  "#3A4252"
    readonly property color text:      "#F2F4F8"
    readonly property color textDim:   "#8B93A4"
    readonly property color textMute:  "#5A6175"

    // Accent — violet to mint gradient. Used sparingly.
    readonly property color accent:    "#9C7CFF"
    readonly property color accentHi:  "#B8A4FF"
    readonly property color accentDim: "#3F2F88"

    // Presence semantics.
    readonly property color online:    "#4FD49B"
    readonly property color away:      "#F2B845"
    readonly property color dnd:       "#F26A6A"
    readonly property color offline:   "#6E7585"
    readonly property color mobile:    "#6FB5FF"

    readonly property color danger:    "#E2495C"
    readonly property color success:   "#4FD49B"

    // Geometry tokens.
    readonly property int radius:   10
    readonly property int radiusSm: 6
    readonly property int radiusLg: 14
    readonly property int gap:      12
    readonly property int gapSm:    8
    readonly property int gapLg:    20
    readonly property int gapXl:    32
    readonly property int padding:  16
    readonly property int paddingLg: 24

    // Type scale.
    readonly property string fontDisplay: Qt.platform.os === "osx" ? "Helvetica Neue" : "Segoe UI"
    readonly property string fontMono:    Qt.platform.os === "osx" ? "Menlo" : "Consolas"

    readonly property int fontXs:  11
    readonly property int fontSm:  12
    readonly property int fontBase: 13
    readonly property int fontMd:  14
    readonly property int fontLg:  16
    readonly property int fontXl:  20
    readonly property int fontXxl: 28
    readonly property int fontHero: 40

    // Motion.
    readonly property int durFast:   120
    readonly property int durMed:    200
    readonly property int durSlow:   320

    function presenceColor(s) {
        if (s === "chat" || s === "online")  return online
        if (s === "away")                    return away
        if (s === "dnd")                     return dnd
        if (s === "mobile")                  return mobile
        return offline
    }
}
