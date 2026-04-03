import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// OmniMountPanel.qml — UI for mapping Bobfilez features to Native OS Drives.
/// Uses Dokany (Windows) or FUSE (Linux) to mount "Live Folders" to actual Drive Letters.

Rectangle {
    id: mountPanel
    color: "#0f0f0f"; radius: 8

    property bool isDriverInstalled: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "💽 OmniMount VFS"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Native OS Drive Mapping (Dokany/FUSE)"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: 150; height: 24; radius: 12; color: mountPanel.isDriverInstalled ? "#1a3a1a" : "#3a1a1a"
                border.color: mountPanel.isDriverInstalled ? "#4caf50" : "#ff4444"
                Label { anchors.centerIn: parent; text: mountPanel.isDriverInstalled ? "Kernel Driver Active" : "Driver Missing"; color: mountPanel.isDriverInstalled ? "#4caf50" : "#ff4444"; font.pixelSize: 10; font.bold: true }
            }
        }

        // ── Active Mounts List ───────────────────────────────────────────
        GroupBox {
            label: Label { text: "Currently Mounted OS Drives"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 8
                model: [
                    {drive: "Z:\\", source: "Semantic Search: 'Invoices 2024'", size: "450 MB", status: "Mounted", ro: false},
                    {drive: "Y:\\", source: "Live Folder: 'Recent Photos'", size: "1.2 GB", status: "Mounted", ro: true},
                    {drive: "X:\\", source: "Data Swarm: 'NAS-PRO' Root", size: "4.8 TB", status: "Syncing Metadata...", ro: false}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 60; radius: 8
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        
                        Rectangle { width: 40; height: 40; radius: 4; color: "#0d0d0d"; border.color: "#0078d4"
                            Label { anchors.centerIn: parent; text: "💽"; font.pixelSize: 20 } }
                        
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: "Local Drive: " + modelData.drive; color: "white"; font.bold: true; font.pixelSize: 13 }
                            Label { text: "Bobfilez Source: " + modelData.source; color: "#888"; font.pixelSize: 11; font.family: "Consolas" }
                        }
                        
                        ColumnLayout {
                            spacing: 4; Layout.preferredWidth: 100
                            Label { text: modelData.status; color: modelData.status === "Mounted" ? "#4caf50" : "#ffaa00"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignRight }
                            Label { text: modelData.ro ? "Read-Only" : "Read/Write"; color: "#666"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                            Label { text: modelData.size; color: "#ccc"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        }
                        
                        Button {
                            text: "Unmount"
                            background: Rectangle { color: "#3a1a1a"; border.color: "#ff4444"; radius: 4 }
                            contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 10; font.bold: true }
                        }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── New Mount Controls ────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Create New Virtual OS Drive"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; height: 120
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            RowLayout {
                anchors.fill: parent; spacing: 15

                ColumnLayout { spacing: 4; Layout.fillWidth: true
                    Label { text: "Bobfilez Virtual Source (Query / Live Folder / Swarm)"; color: "#888"; font.pixelSize: 11 }
                    TextField { placeholderText: "e.g., search: 'budget 2024'"; Layout.fillWidth: true; color: "white"
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#444" } }
                }

                ColumnLayout { spacing: 4; Layout.preferredWidth: 80
                    Label { text: "Drive Letter"; color: "#888"; font.pixelSize: 11 }
                    ComboBox { model: ["W:\\", "V:\\", "U:\\", "T:\\"]; Layout.fillWidth: true
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#444" }
                        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 12 } }
                }

                ColumnLayout { spacing: 4; Layout.preferredWidth: 100
                    Label { text: "Permissions"; color: "#888"; font.pixelSize: 11 }
                    ComboBox { model: ["Read/Write", "Read-Only"]; Layout.fillWidth: true
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#444" }
                        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 12 } }
                }

                Button {
                    text: "Mount Drive"
                    Layout.preferredHeight: 36
                    background: Rectangle { color: "#0078d4"; radius: 6 }
                    contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                }
            }
        }
    }
}
