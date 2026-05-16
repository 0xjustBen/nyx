import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left column: hero + mode orbit + launch bar.
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            Rectangle {
                anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
                width: 1; color: Theme.line
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: 40
                anchors.rightMargin: 40
                anchors.topMargin: 28
                anchors.bottomMargin: 0
                spacing: 0

                // Breadcrumb.
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        text: "PRESENCE"
                        color: Theme.cyan
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontXs
                        font.letterSpacing: 3
                    }
                    Label { text: "/"; color: Theme.muted; opacity: 0.4 }
                    Label {
                        text: "CURRENT STATE"
                        color: Theme.muted
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontXs
                        font.letterSpacing: 3
                    }
                    Label { text: "/"; color: Theme.muted; opacity: 0.4 }
                    Label {
                        text: "AUTO-BROADCAST"
                        color: Theme.muted
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontXs
                        font.letterSpacing: 3
                    }
                    Item { Layout.fillWidth: true }
                }

                // Hero with concentric rings + core.
                Item {
                    id: heroArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 360

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 0

                        Item {
                            id: ringWrap
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 260
                            Layout.preferredHeight: 260
                            Layout.bottomMargin: 24

                            property color accent: Theme.modeColor(App.mode)

                            // Ring-morph: pulse scale + opacity on mode change.
                            Connections {
                                target: App
                                function onModeChanged() {
                                    morphAnim.restart()
                                }
                            }
                            SequentialAnimation {
                                id: morphAnim
                                NumberAnimation { target: ringWrap; property: "scale"; from: 0.92; to: 1.0; duration: Theme.durMed; easing.type: Easing.OutBack }
                            }

                            // Outer ring.
                            Rectangle {
                                anchors.fill: parent
                                radius: width / 2
                                color: "transparent"
                                border.color: Theme.line2
                                border.width: 1
                            }
                            // Dashed ring (approx via opacity).
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 18
                                radius: width / 2
                                color: "transparent"
                                border.color: Theme.line
                                border.width: 1
                                opacity: 0.6
                            }
                            // Glow ring (radial via gradient).
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 42
                                radius: width / 2
                                color: Qt.rgba(ringWrap.accent.r, ringWrap.accent.g, ringWrap.accent.b, 0.18)
                                opacity: 0.7
                            }
                            // Inner ring.
                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 42
                                radius: width / 2
                                color: "transparent"
                                border.color: Qt.rgba(ringWrap.accent.r, ringWrap.accent.g, ringWrap.accent.b, 0.5)
                                border.width: 1
                            }
                            // Core.
                            Rectangle {
                                id: core
                                anchors.fill: parent
                                anchors.margins: 64
                                radius: width / 2
                                gradient: Gradient {
                                    orientation: Gradient.Vertical
                                    GradientStop { position: 0.0; color: Qt.rgba(ringWrap.accent.r, ringWrap.accent.g, ringWrap.accent.b, 0.25) }
                                    GradientStop { position: 1.0; color: Theme.bg }
                                }
                                border.color: Qt.rgba(ringWrap.accent.r, ringWrap.accent.g, ringWrap.accent.b, 0.5)
                                border.width: 1
                                Behavior on border.color { ColorAnimation { duration: Theme.durMed } }

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    spacing: 4
                                    Label {
                                        text: Theme.modeGlyph(App.mode)
                                        color: ringWrap.accent
                                        font.family: Theme.fontDisplay
                                        font.pixelSize: Theme.fontGlyph
                                        font.weight: Font.DemiBold
                                        Layout.alignment: Qt.AlignHCenter
                                        Behavior on color { ColorAnimation { duration: Theme.durMed } }
                                    }
                                    Label {
                                        text: (App.mode === "offline" ? "Invisible" : App.mode).toUpperCase()
                                        color: Theme.muted2
                                        font.family: Theme.fontDisplay
                                        font.pixelSize: Theme.fontXs
                                        font.letterSpacing: 4
                                        Layout.alignment: Qt.AlignHCenter
                                    }
                                }
                            }
                        }

                        Label {
                            text: "FRIENDS CURRENTLY SEE YOU AS"
                            color: Theme.muted
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontMd
                            font.letterSpacing: 3
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Label {
                            text: Theme.modeFull(App.mode)
                            color: Theme.fg
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontHero
                            font.weight: Font.Bold
                            font.letterSpacing: -0.5
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 10
                        }
                        Label {
                            text: Theme.modeDesc(App.mode)
                            color: Theme.muted2
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontBase
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 400
                            Layout.topMargin: 8
                        }
                    }
                }

                // Mode orbit (5 pills).
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 8
                    spacing: 10
                    ModePill { mode: "online";  label: "Online";    kbd: "⌘ 1" }
                    ModePill { mode: "away";    label: "Away";      kbd: "⌘ 2" }
                    ModePill { mode: "mobile";  label: "Mobile";    kbd: "⌘ 3" }
                    ModePill { mode: "dnd";     label: "DND";       kbd: "⌘ 4" }
                    ModePill { mode: "offline"; label: "Invisible"; kbd: "⌘ 5" }
                }

                // Launch bar.
                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: 24
                    Layout.preferredHeight: 60
                    color: "transparent"
                    Rectangle {
                        anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
                        height: 1; color: Theme.line
                    }
                    RowLayout {
                        anchors.fill: parent
                        anchors.topMargin: 14
                        anchors.bottomMargin: 14
                        spacing: 8
                        Label {
                            text: "LAUNCH WITH NYX ACTIVE"
                            color: Theme.muted
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 3
                            Layout.fillWidth: true
                        }
                        NeonBtn { text: "Install cert";   onClicked: App.installCert() }
                        NeonBtn { text: "Patch hosts";    onClicked: App.patchHostsFile() }
                        NeonBtn { text: "League";         onClicked: App.launchProduct("league") }
                        NeonBtn { text: "Valorant";       onClicked: App.launchProduct("valorant") }
                        NeonBtn { text: "▶ Riot Client";  primary: true; onClicked: App.launchRiot() }
                    }
                }
            }
        }

        // Right column: roster.
        Rectangle {
            Layout.preferredWidth: 360
            Layout.fillHeight: true
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Roster head.
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    color: "transparent"
                    Rectangle {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                        height: 1; color: Theme.line
                    }
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        spacing: 10
                        Label {
                            text: "ROSTER"
                            color: Theme.muted
                            font.family: Theme.fontDisplay
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 3
                            font.bold: true
                        }
                        Label {
                            text: App.roster.rowCount() + " · live"
                            color: Theme.cyan
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontXs
                            font.letterSpacing: 1
                        }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: "⌕"
                            color: Theme.muted
                            font.pixelSize: Theme.fontLg
                            opacity: 0.6
                        }
                    }
                }

                // Tabs.
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    color: "transparent"
                    Rectangle {
                        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                        height: 1; color: Theme.line
                    }
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        spacing: 0

                        property string current: "All"
                        id: tabsRow

                        Repeater {
                            model: ["All", "Online", "In-Game", "Recent"]
                            delegate: Item {
                                Layout.preferredHeight: 36
                                Layout.preferredWidth: tabLabel.implicitWidth + 24
                                Label {
                                    id: tabLabel
                                    anchors.centerIn: parent
                                    text: modelData.toUpperCase()
                                    color: tabsRow.current === modelData ? Theme.cyan : Theme.muted
                                    font.family: Theme.fontDisplay
                                    font.pixelSize: Theme.fontXs
                                    font.letterSpacing: 2
                                }
                                Rectangle {
                                    anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                                    height: 2
                                    color: tabsRow.current === modelData ? Theme.cyan : "transparent"
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: tabsRow.current = modelData
                                }
                            }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }

                // Roster list — show demo entries if empty.
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 0
                    topMargin: 8
                    bottomMargin: 8

                    // Show real roster if populated, else fall back to demo
                    // entries so the empty UI doesn't look broken.
                    model: App.roster && App.roster.rowCount() > 0 ? App.roster : demoRosterModel
                    delegate: FriendRow {
                        width: ListView.view.width
                        friendName: model.name || ""
                        friendTag: (typeof model.tag !== "undefined" ? model.tag : "")
                        friendActivity: (typeof model.activity !== "undefined" && model.activity.length > 0)
                                      ? model.activity
                                      : (model.presence || "")
                        friendGame: (typeof model.game !== "undefined" ? model.game : "")
                        friendPresence: model.presence || "offline"
                    }
                }

                ListModel {
                    id: demoRosterModel
                    ListElement { name: "Faker";       tag: "#KR1";  activity: "In ranked solo · 23m";    game: "LOL"; presence: "online" }
                    ListElement { name: "TenZ";        tag: "#NA1";  activity: "Haven · Sage · 1-0";       game: "VAL"; presence: "online" }
                    ListElement { name: "midbeast";    tag: "#EUW";  activity: "Do not disturb";           game: "LOL"; presence: "dnd" }
                    ListElement { name: "shroud";      tag: "#NA1";  activity: "Custom · Ascent";          game: "VAL"; presence: "online" }
                    ListElement { name: "kobo_x";      tag: "#EUW";  activity: "Mobile · 4m ago";          game: "2XKO"; presence: "mobile" }
                    ListElement { name: "veil";        tag: "#NA1";  activity: "Away · in lobby";          game: "LOR"; presence: "away" }
                    ListElement { name: "noxhide";     tag: "#EUW";  activity: "Champion select";          game: "LOL"; presence: "online" }
                    ListElement { name: "ravnsdotter"; tag: "#EUW";  activity: "Offline · 2h";             game: "LOL"; presence: "offline" }
                }
            }
        }
    }
}
