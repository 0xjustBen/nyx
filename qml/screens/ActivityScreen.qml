import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx
//- need clipboard


Rectangle {
    color: Theme.bg

    property string filter: "All"

    ListModel { id: events }

    Component.onCompleted: {
        // Seed with a handful of synthetic entries — overwritten by real logLine signal.
        const seed = [
            ["14:08:42.118", "presence", "Broadcast state <em>Online</em> → 127 friends · XMPP roster push <dim>[seq 0x4af2]</dim>"],
            ["14:08:42.097", "system",   "Mode switch <dim>Invisible → Online</dim> via ⌘1 hotkey"],
            ["14:07:53.882", "friend",   "<em>TenZ#NA1</em> entered match · Valorant · Haven · Sage"],
            ["14:07:21.014", "friend",   "<em>Faker#KR1</em> presence changed: Lobby → In Ranked Solo"],
            ["14:05:09.776", "game",     "Launched <em>Riot Client</em> with Nyx proxy attached · pid 41872"],
            ["14:04:58.302", "system",   "Mode switch <dim>Online → Invisible</dim>"],
            ["14:03:11.045", "friend",   "<em>kobo_x#EUW</em> went mobile (Riot Mobile · 2XKO)"],
            ["14:02:48.901", "presence", "Roster sync · 127 entries · 41 online · delta +3"],
            ["14:01:30.667", "system",   "XMPP connection established · euw1.chat.si.riotgames.com:5223 · TLS 1.3"],
            ["14:01:29.114", "system",   "Local proxy bound on <em>127.0.0.1:23170</em>"],
            ["14:01:28.502", "system",   "Nyx started · v 0.1.0-alpha"],
        ];
        for (const [t, tag, msg] of seed) events.append({ ts: t, tag: tag, msg: msg });
    }

    Connections {
        target: App
        function onLogLine(line) {
            const ts = Qt.formatTime(new Date(), "HH:mm:ss.zzz")
            events.insert(0, { ts: ts, tag: "system", msg: line })
            if (events.count > 500) events.remove(events.count - 1)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Head.
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "transparent"
            Rectangle {
                anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                height: 1; color: Theme.line
            }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 28
                anchors.rightMargin: 28
                spacing: 16
                Label {
                    text: "Activity"
                    color: Theme.fg
                    font.family: Theme.fontDisplay
                    font.pixelSize: 18
                    font.weight: Font.Bold
                }
                Label {
                    text: events.count + " events"
                    color: Theme.cyan
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontSm
                    font.letterSpacing: 1
                }
                Item { Layout.fillWidth: true }
                NeonBtn {
                    text: "Copy log"
                    onClicked: {
                        let txt = ""
                        for (let i = events.count - 1; i >= 0; --i) {
                            const r = events.get(i)
                            txt += r.ts + "  [" + r.tag + "]  " +
                                   r.msg.replace(/<\/?(em|dim)>/g, "") + "\n"
                        }
                        clipboardHelper.text = txt
                        clipboardHelper.selectAll()
                        clipboardHelper.copy()
                    }
                }
                TextEdit { id: clipboardHelper; visible: false }
                Repeater {
                    model: ["All","Presence","Friends","Game","System"]
                    delegate: Rectangle {
                        property bool on: filter === modelData
                        implicitHeight: 22
                        implicitWidth: filterLbl.implicitWidth + 20
                        radius: 99
                        color: on ? Qt.rgba(0.337, 0.800, 0.878, 0.08) : "transparent"
                        border.color: on ? Theme.cyan : Theme.line
                        border.width: 1
                        Label {
                            id: filterLbl
                            anchors.centerIn: parent
                            text: modelData.toUpperCase()
                            color: parent.on ? Theme.cyan : Theme.muted
                            font.family: Theme.fontDisplay
                            font.pixelSize: 9
                            font.letterSpacing: 2
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: filter = modelData
                        }
                    }
                }
            }
        }

        // Log rows.
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: events
            spacing: 0
            delegate: Rectangle {
                width: ListView.view.width
                height: 28
                color: rowMa.containsMouse ? Theme.surface : "transparent"
                visible: filter === "All" || filter.toLowerCase() === model.tag

                MouseArea { id: rowMa; anchors.fill: parent; hoverEnabled: true }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 28
                    anchors.rightMargin: 28
                    spacing: 16

                    Label {
                        text: model.ts
                        color: Theme.muted
                        font.family: Theme.fontMono
                        font.pixelSize: Theme.fontXs
                        Layout.preferredWidth: 120
                    }

                    Rectangle {
                        Layout.preferredWidth: 70
                        Layout.preferredHeight: 16
                        radius: 3
                        color: "transparent"
                        border.color: model.tag === "presence" ? Theme.cyan
                                    : model.tag === "friend"   ? Theme.magenta
                                    : model.tag === "game"     ? Theme.amber
                                                               : Theme.muted2
                        border.width: 1
                        Label {
                            anchors.centerIn: parent
                            text: model.tag.toUpperCase()
                            color: model.tag === "presence" ? Theme.cyan
                                 : model.tag === "friend"   ? Theme.magenta
                                 : model.tag === "game"     ? Theme.amber
                                                            : Theme.muted2
                            font.family: Theme.fontDisplay
                            font.pixelSize: 9
                            font.letterSpacing: 2
                        }
                    }

                    Label {
                        text: model.msg.replace(/<em>/g, "<font color='" + Theme.cyan + "'>")
                                       .replace(/<\/em>/g, "</font>")
                                       .replace(/<dim>/g, "<font color='" + Theme.muted + "'>")
                                       .replace(/<\/dim>/g, "</font>")
                        textFormat: Text.RichText
                        color: Theme.fg
                        font.family: Theme.fontMono
                        font.pixelSize: Theme.fontSm
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
