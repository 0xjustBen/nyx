pragma Singleton
import QtQuick

// Neon-noir palette per open-design output. OKLch → hex approximations
// (Qt doesn't accept oklch literals).
QtObject {
    readonly property color bgDeep:   "#080B11"   // oklch(7% 0.012 240)
    readonly property color bg:       "#0E1119"   // oklch(10% 0.012 240)
    readonly property color surface:  "#161A23"   // oklch(14% 0.018 240)
    readonly property color surface2: "#1C212C"   // oklch(17% 0.022 240)
    readonly property color line:     "#232936"   // oklch(20% 0.025 240)
    readonly property color line2:    "#2F3645"   // oklch(26% 0.028 240)
    readonly property color muted:    "#828A9A"   // oklch(58% 0.025 240)
    readonly property color muted2:   "#A4ABBA"   // oklch(70% 0.025 240)
    readonly property color fg:       "#ECF2F5"   // oklch(96% 0.012 200)

    readonly property color cyan:     "#56CCE0"   // oklch(78% 0.16 200)
    readonly property color cyanDim:  "#38939F"   // oklch(58% 0.14 200)
    readonly property color magenta:  "#E057AC"   // oklch(68% 0.24 340)
    readonly property color amber:    "#E8B45D"   // oklch(82% 0.18 80)
    readonly property color green:    "#56D096"   // oklch(78% 0.18 145)
    readonly property color red:      "#E26956"   // oklch(68% 0.22 25)
    readonly property color grey:     "#8A909C"   // oklch(60% 0.012 240)

    // Game-specific brand tints.
    readonly property color gameLol:  "#E0B85D"
    readonly property color gameVal:  "#E26956"
    readonly property color gameLor:  "#56CCE0"
    readonly property color game2xko: "#E057AC"

    // Cyan with alpha — use Qt.rgba based off cyan channel values.
    function cyanAlpha(a) { return Qt.rgba(0.337, 0.800, 0.878, a) }
    function redAlpha(a)  { return Qt.rgba(0.886, 0.412, 0.337, a) }

    readonly property int rSm: 4
    readonly property int r:   8
    readonly property int rLg: 14

    readonly property string fontDisplay: Qt.platform.os === "osx" ? "Helvetica Neue" : "Segoe UI"
    readonly property string fontMono:    Qt.platform.os === "osx" ? "Menlo" : "Consolas"

    // Type scale (pixel px).
    readonly property int fontXs:    10
    readonly property int fontSm:    11
    readonly property int fontBase:  12
    readonly property int fontMd:    13
    readonly property int fontLg:    14
    readonly property int fontXl:    18
    readonly property int fontHero:  32
    readonly property int fontGlyph: 42

    readonly property int durFast: 150
    readonly property int durMed:  220
    readonly property int durSlow: 500

    function presenceColor(s) {
        if (s === "chat" || s === "online")  return green
        if (s === "away")                    return amber
        if (s === "dnd")                     return red
        if (s === "mobile")                  return magenta
        return grey
    }

    function modeColor(m) {
        if (m === "online" || m === "chat")  return cyan
        if (m === "away")                    return amber
        if (m === "mobile")                  return magenta
        if (m === "dnd")                     return red
        return grey
    }

    function modeGlyph(m) {
        if (m === "online" || m === "chat")  return "●"
        if (m === "away")                    return "◐"
        if (m === "mobile")                  return "◇"
        if (m === "dnd")                     return "✕"
        return "○"
    }

    function modeFull(m) {
        if (m === "online" || m === "chat")  return "Online — In Lobby"
        if (m === "away")                    return "Away — Stepped out"
        if (m === "mobile")                  return "Mobile — Riot Mobile"
        if (m === "dnd")                     return "Do Not Disturb"
        return "Invisible — Hidden"
    }

    function modeDesc(m) {
        if (m === "online" || m === "chat")  return "Friends see your real status across all Riot titles."
        if (m === "away")                    return "Friends see you idle. Queue auto-accept disabled."
        if (m === "mobile")                  return "Spoof Riot Mobile presence. Friends think you're on the go."
        if (m === "dnd")                     return "Notifications muted. Friends get a clear 'busy' signal."
        return "Nyx drops your presence packets. You appear fully offline."
    }
}
