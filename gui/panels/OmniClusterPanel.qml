import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// OmniClusterPanel.qml — Distributed Grid Computing Manager.
/// Distribute heavy tasks (FFmpeg, Hashcat, OCR) across multiple
/// Bobfilez nodes on your local network.

Rectangle {
    id: clusterPanel
    color: "#050505"; radius: 8

    property bool isDispatching: false
    property int activeNodes: 2

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "⚡ OmniCluster Grid Computing"; font.pixelSize: 22; font.bold: true; color: "#4caf50" }
            Label { text: "Distributed Processing Farm"; color: "#888"; font.pixelSize: 14 }
            
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: 140; height: 24; radius: 12; color: "#1a3a1a"; border.color: "#4caf50"
                Label { anchors.centerIn: parent; text: "SWARM ACTIVE: " + clusterPanel.activeNodes; color: "#4caf50"; font.pixelSize: 10; font.bold: true }
            }
        }

        // ── Main Content Area ─────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 20

            // 1. Available Compute Nodes (Left)
            GroupBox {
                label: Label { text: "Available Compute Nodes"; color: "#aaa"; font.bold: true }
                Layout.preferredWidth: 350; Layout.fillHeight: true
                background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                padding: 10

                ListView {
                    anchors.fill: parent; clip: true; spacing: 8
                    model: [
                        {host: "Main Desktop (You)", cpu: "16-Core Ryzen 9", gpu: "RTX 4090", load: 0.15, isOnline: true},
                        {host: "Laptop X1 Carbon", cpu: "8-Core Intel i7", gpu: "Iris Xe", load: 0.40, isOnline: true},
                        {host: "TrueNAS Storage", cpu: "4-Core Celeron", gpu: "None", load: 0.85, isOnline: false}
                    ]
                    
                    delegate: Rectangle {
                        width: parent.width; height: 80; radius: 6
                        color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                        
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 10; spacing: 10
                            
                            Rectangle { width: 40; height: 40; radius: 4; color: modelData.isOnline ? "#1a3a1a" : "#3a1a1a"; border.color: modelData.isOnline ? "#4caf50" : "#ff4444"
                                Label { anchors.centerIn: parent; text: "💻"; font.pixelSize: 20 } }
                            
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Label { text: modelData.host; color: "white"; font.pixelSize: 12; font.bold: true }
                                Label { text: "CPU: " + modelData.cpu + " | GPU: " + modelData.gpu; color: "#888"; font.pixelSize: 10 }
                                
                                // Load Bar
                                RowLayout {
                                    Label { text: "Load:"; color: "#666"; font.pixelSize: 9 }
                                    ProgressBar { value: modelData.load; Layout.fillWidth: true; 
                                        background: Rectangle { color: "#222"; radius: 2; implicitHeight: 4 }
                                        contentItem: Rectangle { color: modelData.load > 0.8 ? "#ff4444" : "#0078d4"; radius: 2 } }
                                    Label { text: Math.round(modelData.load * 100) + "%"; color: "#888"; font.pixelSize: 9; width: 25 }
                                }
                            }
                        }
                        HoverHandler { id: itemHover }
                    }
                }
            }

            // 2. Active Distributed Tasks (Right)
            GroupBox {
                label: Label { text: "Active Distributed Tasks"; color: "#aaa"; font.bold: true }
                Layout.fillWidth: true; Layout.fillHeight: true
                background: Rectangle { color: "#111"; radius: 6; border.color: "#333" }
                padding: 10

                ColumnLayout {
                    anchors.fill: parent; spacing: 15

                    // Render Task
                    Rectangle {
                        Layout.fillWidth: true; height: 100; radius: 8; color: "#1a1a1a"; border.color: "#444"
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 15; spacing: 8
                            
                            RowLayout {
                                Label { text: "🎬 OmniVision: Rendering 'Holiday_Edit_v2.mp4' (4K HEVC)"; color: "white"; font.bold: true; font.pixelSize: 13; Layout.fillWidth: true }
                                Label { text: "45 / 120 Chunks"; color: "#0078d4"; font.pixelSize: 11; font.family: "Consolas" }
                                Button { text: "⏹"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 14 } }
                            }

                            // Sub-Nodes Status
                            RowLayout {
                                spacing: 10; Layout.fillWidth: true
                                Rectangle { width: 120; height: 20; color: "#252525"; radius: 4
                                    Label { anchors.centerIn: parent; text: "Desktop: Chunk 46"; color: "#4caf50"; font.pixelSize: 9 } }
                                Rectangle { width: 120; height: 20; color: "#252525"; radius: 4
                                    Label { anchors.centerIn: parent; text: "Laptop: Chunk 47"; color: "#4caf50"; font.pixelSize: 9 } }
                                Item { Layout.fillWidth: true }
                                Label { text: "Swarm Speed: 84.5 MB/s"; color: "#888"; font.pixelSize: 10 }
                            }

                            ProgressBar {
                                value: 45/120; Layout.fillWidth: true
                                background: Rectangle { color: "#222"; radius: 4; implicitHeight: 8 }
                                contentItem: Rectangle { color: "#0078d4"; radius: 4 }
                            }
                        }
                    }

                    // Hashcat Task
                    Rectangle {
                        Layout.fillWidth: true; height: 100; radius: 8; color: "#1a1a1a"; border.color: "#444"
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 15; spacing: 8
                            
                            RowLayout {
                                Label { text: "☠️ OmniSec: Cracking 'wallet_backup.7z'"; color: "white"; font.bold: true; font.pixelSize: 13; Layout.fillWidth: true }
                                Label { text: "Keyspace: 35%"; color: "#ffaa00"; font.pixelSize: 11; font.family: "Consolas" }
                                Button { text: "⏹"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 14 } }
                            }

                            RowLayout {
                                spacing: 10; Layout.fillWidth: true
                                Rectangle { width: 140; height: 20; color: "#252525"; radius: 4
                                    Label { anchors.centerIn: parent; text: "RTX 4090: 28,000 kH/s"; color: "#ffaa00"; font.pixelSize: 9 } }
                                Rectangle { width: 140; height: 20; color: "#252525"; radius: 4
                                    Label { anchors.centerIn: parent; text: "Iris Xe: 3,200 kH/s"; color: "#ffaa00"; font.pixelSize: 9 } }
                                Item { Layout.fillWidth: true }
                            }

                            ProgressBar {
                                value: 0.35; Layout.fillWidth: true
                                background: Rectangle { color: "#222"; radius: 4; implicitHeight: 8 }
                                contentItem: Rectangle { color: "#ffaa00"; radius: 4 }
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
        }

        // ── Footer ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Button {
                text: "✨ Distribute Load Automatically (Nexus)"
                background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true }
            }
            Item { Layout.fillWidth: true }
            Button { text: "⚙️ Cluster Settings"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
        }
    }
}
