import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// StartMenu.qml — AI-powered Windows 11 style start menu.
/// Features a search bar, pinned apps, and AI "Recommended" section.

Rectangle {
    id: startMenu
    width: 600; height: 700
    radius: 8
    color: "#e61a1a1a" // Acrylic dark
    border.color: "#33ffffff"
    clip: true

    property string userName: "Robert"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // ── Search Bar ────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 36; radius: 18
            color: "#33000000"; border.color: "#33ffffff"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 12; spacing: 8
                Label { text: "🔍"; color: "#888" }
                TextInput {
                    Layout.fillWidth: true; color: "white"; font.pixelSize: 14
                    placeholderText: "Search for apps, settings, and files"; placeholderTextColor: "#666"
                }
            }
        }

        // ── Pinned Apps ───────────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true; spacing: 10
            RowLayout {
                Label { text: "Pinned"; color: "white"; font.bold: true; font.pixelSize: 13 }
                Item { Layout.fillWidth: true }
                Button { text: "All apps >"; flat: true; contentItem: Label { text: parent.text; color: "#aaa"; font.pixelSize: 11 } }
            }

            GridLayout {
                columns: 6; columnSpacing: 10; rowSpacing: 20; Layout.fillWidth: true
                
                Repeater {
                    model: [
                        {name: "Filez Explorer", icon: "📁", id: "explorer"},
                        {name: "Visual Dedup", icon: "🔥", id: "visual_dedup"},
                        {name: "Batch Convert", icon: "🔄", id: "convert"},
                        {name: "Hex Editor", icon: "🔢", id: "hex"},
                        {name: "Search", icon: "🔍", id: "search"},
                        {name: "File Watcher", icon: "👁️", id: "watcher"},
                        {name: "Settings", icon: "⚙️", id: "settings"},
                        {name: "Browser", icon: "🌐", id: "browser"},
                        {name: "Terminal", icon: "⌨️", id: "terminal"},
                        {name: "Sync", icon: "🔁", id: "fileops"},
                        {name: "Diff", icon: "⚖️", id: "fileops"},
                        {name: "Backup", icon: "💾", id: "fileops"}
                    ]
                    
                    ColumnLayout {
                        Layout.preferredWidth: 80; spacing: 6
                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter; width: 40; height: 40; radius: 4
                            color: hoverApp.hovered ? "#22ffffff" : "transparent"
                            Label { anchors.centerIn: parent; text: modelData.icon; font.pixelSize: 24 }
                            HoverHandler { id: hoverApp }
                            MouseArea { anchors.fill: parent; onClicked: shell.openPanel(modelData.id) }
                        }
                        Label { text: modelData.name; color: "white"; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter; elide: Text.ElideRight; Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter }
                    }
                }
            }
        }

        // ── Recommended (AI Powered) ───────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10
            RowLayout {
                Label { text: "Recommended"; color: "white"; font.bold: true; font.pixelSize: 13 }
                Item { Layout.fillWidth: true }
                Button { text: "More >"; flat: true; contentItem: Label { text: parent.text; color: "#aaa"; font.pixelSize: 11 } }
            }

            GridLayout {
                columns: 2; columnSpacing: 20; rowSpacing: 8; Layout.fillWidth: true
                
                Repeater {
                    model: [
                        {name: "project_plan.md", sub: "Modified 2h ago", icon: "📝"},
                        {name: "IMG_9821.JPG", sub: "Recently added", icon: "📷"},
                        {name: "Presentation.pptx", sub: "Opened yesterday", icon: "📊"},
                        {name: "source_code.cpp", sub: "Modified 5m ago", icon: "📄"},
                        {name: "vacation_video.mp4", sub: "Recently converted", icon: "🎬"},
                        {name: "invoice_2024.pdf", sub: "Found via Search", icon: "📄"}
                    ]
                    
                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
                        Rectangle { width: 32; height: 32; radius: 4; color: "#11ffffff"
                            Label { anchors.centerIn: parent; text: modelData.icon; font.pixelSize: 16 } }
                        Column {
                            Label { text: modelData.name; color: "white"; font.pixelSize: 12 }
                            Label { text: modelData.sub; color: "#888"; font.pixelSize: 10 }
                        }
                    }
                }
            }
        }

        // ── Footer ────────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 60; radius: 4
            color: "#11ffffff"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20
                
                // Profile
                RowLayout {
                    spacing: 10
                    Rectangle { width: 32; height: 32; radius: 16; color: "#0078d4"
                        Label { anchors.centerIn: parent; text: startMenu.userName[0]; color: "white"; font.bold: true } }
                    Label { text: startMenu.userName; color: "white"; font.pixelSize: 12 }
                }

                Item { Layout.fillWidth: true }

                // Power button
                Rectangle {
                    width: 32; height: 32; radius: 4
                    color: hoverPower.hovered ? "#22ffffff" : "transparent"
                    Label { anchors.centerIn: parent; text: "⏻"; color: "white"; font.pixelSize: 18 }
                    HoverHandler { id: hoverPower }
                }
            }
        }
    }
}
