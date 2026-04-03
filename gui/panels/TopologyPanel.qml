import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// TopologyPanel.qml — Visual "Data Topology" Map (Tree-map).
/// Visualizes disk usage using nested rectangles where area = file size.
/// Inspired by WinDirStat, SequoiaView, and DaisyDisk.

Rectangle {
    id: topologyPanel
    color: "#0a0a0a"; radius: 8

    property string rootPath: "C:/"
    property var topologyData: [] // [{name, size, type, children: [...]}]
    property bool isScanning: false
    property var selectedItem: null

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🌲 Data Topology"; font.pixelSize: 20; font.bold: true; color: "white" }
            
            Rectangle {
                Layout.fillWidth: true; height: 32; radius: 6; color: "#1a1a1a"; border.color: "#333"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10
                    Label { text: "📁"; color: "#888" }
                    TextInput { text: topologyPanel.rootPath; color: "white"; Layout.fillWidth: true; font.pixelSize: 12 }
                }
            }

            Button {
                text: topologyPanel.isScanning ? "⏳ Scanning..." : "▶ Analyze Disk"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: topologyPanel.isScanning = !topologyPanel.isScanning
            }
        }

        // ── Main Map Area ──────────────────────────────────────────────────
        Item {
            Layout.fillWidth: true; Layout.fillHeight: true

            // Simple recursive Tree-map renderer
            // In a real implementation, this would be a custom QQuickItem (C++)
            // but for the UI mockup, we use a Repeater.
            
            Rectangle {
                anchors.fill: parent; color: "#050505"; border.color: "#222"
                clip: true

                // Mock treemap nodes
                Row {
                    anchors.fill: parent; spacing: 1
                    
                    // Huge files block
                    Rectangle {
                        width: parent.width * 0.6; height: parent.height; color: "#1a1a1a"; border.color: "#333"
                        Column {
                            anchors.fill: parent; spacing: 1
                            Rectangle { width: parent.width; height: parent.height * 0.7; color: "#2d3a4a"; border.color: "#4a9eff"
                                Label { anchors.centerIn: parent; text: "System (24 GB)"; color: "white"; font.pixelSize: 10 } }
                            Row {
                                width: parent.width; height: parent.height * 0.3; spacing: 1
                                Rectangle { width: parent.width * 0.5; height: parent.height; color: "#3a2a1a"; border.color: "#ffaa00"
                                    Label { anchors.centerIn: parent; text: "Videos (8 GB)"; color: "white"; font.pixelSize: 9 } }
                                Rectangle { width: parent.width * 0.5; height: parent.height; color: "#1a3a1a"; border.color: "#4caf50"
                                    Label { anchors.centerIn: parent; text: "Photos (6 GB)"; color: "white"; font.pixelSize: 9 } }
                            }
                        }
                    }

                    // Remaining space
                    Column {
                        width: parent.width * 0.4; height: parent.height; spacing: 1
                        Rectangle { width: parent.width; height: parent.height * 0.4; color: "#2a1a3a"; border.color: "#8b5cf6"
                            Label { anchors.centerIn: parent; text: "Apps (4 GB)"; color: "white"; font.pixelSize: 9 } }
                        
                        Grid {
                            columns: 4; width: parent.width; height: parent.height * 0.6; spacing: 1
                            Repeater {
                                model: 16
                                Rectangle {
                                    width: (parent.width - 3) / 4; height: (parent.height - 3) / 4
                                    color: Qt.hsla(Math.random(), 0.5, 0.2, 1.0)
                                    border.color: Qt.lighter(color)
                                    opacity: hoverNode.hovered ? 1.0 : 0.7
                                    HoverHandler { id: hoverNode }
                                    ToolTip.visible: hoverNode.hovered; ToolTip.text: "Misc File " + index + "\nSize: 245 MB"
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Details Footer ────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 100; color: "#161616"; radius: 6; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.margins: 15; spacing: 20

                // Large Type Breakdown
                ColumnLayout {
                    Label { text: "Type Breakdown"; color: "#888"; font.pixelSize: 11; font.bold: true }
                    Row {
                        spacing: 4
                        Rectangle { width: 40; height: 10; color: "#4a9eff" }; Label { text: "System"; color: "#ccc"; font.pixelSize: 10 }
                        Rectangle { width: 40; height: 10; color: "#ffaa00" }; Label { text: "Video"; color: "#ccc"; font.pixelSize: 10 }
                        Rectangle { width: 40; height: 10; color: "#4caf50" }; Label { text: "Image"; color: "#ccc"; font.pixelSize: 10 }
                        Rectangle { width: 40; height: 10; color: "#8b5cf6" }; Label { text: "Apps"; color: "#ccc"; font.pixelSize: 10 }
                    }
                }

                Item { Layout.fillWidth: true }

                // Selection Info
                ColumnLayout {
                    Layout.preferredWidth: 300
                    Label { text: "Selected: /System/Library/Caches/..."; color: "white"; font.pixelSize: 12; elide: Text.ElideLeft; Layout.fillWidth: true }
                    Label { text: "Size: 1.2 GB  |  Files: 4,210  |  Modified: 2h ago"; color: "#888"; font.pixelSize: 11 }
                    RowLayout {
                        Button { text: "🗑 Delete"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 11 } }
                        Button { text: "📂 Open"; flat: true; contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 11 } }
                        Button { text: "✨ Clean"; flat: true; contentItem: Label { text: parent.text; color: "#4caf50"; font.pixelSize: 11 } }
                    }
                }
            }
        }
    }
}
