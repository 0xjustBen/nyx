import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Nyx

Rectangle {
    color: Theme.bg

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.parent.availableWidth
            anchors.margins: Theme.paddingLg
            spacing: Theme.gapXl

            ColumnLayout {
                Layout.topMargin: Theme.paddingLg
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                spacing: 4
                Label {
                    text: "SETTINGS"
                    color: Theme.textMute
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 3
                }
                Label {
                    text: "Preferences"
                    color: Theme.text
                    font.family: Theme.fontDisplay
                    font.pixelSize: Theme.fontXxl
                    font.weight: Font.DemiBold
                }
            }

            // Section: Connection
            ColumnLayout {
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                Layout.fillWidth: true
                spacing: Theme.gap

                Label {
                    text: "CONNECTION"
                    color: Theme.textMute
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 2
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: regionCol.implicitHeight + Theme.paddingLg * 2
                    radius: Theme.radius
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1

                    ColumnLayout {
                        id: regionCol
                        anchors.fill: parent
                        anchors.margins: Theme.paddingLg
                        spacing: Theme.gap

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                spacing: 2
                                Label {
                                    text: "Region"
                                    color: Theme.text
                                    font.pixelSize: Theme.fontMd
                                    font.weight: Font.Medium
                                }
                                Label {
                                    text: "Default chat host before clientconfig resolves"
                                    color: Theme.textDim
                                    font.pixelSize: Theme.fontSm
                                }
                            }
                            Item { Layout.fillWidth: true }
                            ComboBox {
                                model: ["na1","euw1","eun1","kr","br1","la1","la2","oc1","ru","tr1","jp1","ph2","sg2","th2","tw2","vn2"]
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.border }

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                spacing: 2
                                Label {
                                    text: "Default mode at startup"
                                    color: Theme.text
                                    font.pixelSize: Theme.fontMd
                                    font.weight: Font.Medium
                                }
                                Label {
                                    text: "What friends see when Nyx launches"
                                    color: Theme.textDim
                                    font.pixelSize: Theme.fontSm
                                }
                            }
                            Item { Layout.fillWidth: true }
                            ComboBox {
                                model: ["online","away","mobile","dnd","offline"]
                                currentIndex: model.indexOf(App.mode)
                                onActivated: App.mode = currentText
                            }
                        }
                    }
                }
            }

            // Section: Advanced
            ColumnLayout {
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                Layout.fillWidth: true
                spacing: Theme.gap

                Label {
                    text: "ADVANCED"
                    color: Theme.textMute
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 2
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: advCol.implicitHeight + Theme.paddingLg * 2
                    radius: Theme.radius
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1

                    ColumnLayout {
                        id: advCol
                        anchors.fill: parent
                        anchors.margins: Theme.paddingLg
                        spacing: Theme.gap

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Label { text: "Start with system"; color: Theme.text; font.pixelSize: Theme.fontMd; font.weight: Font.Medium }
                                Label { text: "Launch Nyx on boot"; color: Theme.textDim; font.pixelSize: Theme.fontSm }
                            }
                            Item { Layout.fillWidth: true }
                            Switch { }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.border }

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Label { text: "Show notifications"; color: Theme.text; font.pixelSize: Theme.fontMd; font.weight: Font.Medium }
                                Label { text: "Toast when friends ping you"; color: Theme.textDim; font.pixelSize: Theme.fontSm }
                            }
                            Item { Layout.fillWidth: true }
                            Switch { checked: true }
                        }
                    }
                }
            }

            // Section: About
            ColumnLayout {
                Layout.leftMargin: Theme.paddingLg
                Layout.rightMargin: Theme.paddingLg
                Layout.bottomMargin: Theme.paddingLg
                Layout.fillWidth: true
                spacing: Theme.gap

                Label {
                    text: "ABOUT"
                    color: Theme.textMute
                    font.family: Theme.fontMono
                    font.pixelSize: Theme.fontXs
                    font.letterSpacing: 2
                }
                Label {
                    text: "Nyx v0.1 alpha · Apache 2.0"
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSm
                }
                Label {
                    text: "Inspired by Deceive. No memory hooks, no game traffic touched."
                    color: Theme.textMute
                    font.pixelSize: Theme.fontXs
                }
            }
        }
    }
}
