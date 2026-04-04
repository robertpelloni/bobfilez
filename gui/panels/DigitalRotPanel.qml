import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// DigitalRotPanel.qml — Autonomous "Digital Rot" Cleanup Agent.
/// Suggests files to prune based on age, redundancy, and file type patterns.
/// Part of the "Data Custodian" suite.

Rectangle {
    id: rotPanel
    color: "#0f0f0f"; radius: 8

    property var suggestions: [] // [{path, reason, confidence, size}]
    property bool isScanning: false
    property real reclaimableBytes: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🧹 Digital Rot Agent"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "AI-Powered System Pruning"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: rotPanel.isScanning ? "⏳ Analyzing..." : "🔍 Scan for Rot"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: rotPanel.isScanning = !rotPanel.isScanning
            }
        }

        // ── Stats Row ──────────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            Rectangle {
                Layout.fillWidth: true; height: 100; radius: 10; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Reclaimable Space"; color: "#888"; font.pixelSize: 11 }
                    Label { text: (rotPanel.reclaimableBytes / (1024*1024*1024)).toFixed(2) + " GB"; color: "#4caf50"; font.pixelSize: 28; font.bold: true }
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 100; radius: 10; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Risk Files Found"; color: "#888"; font.pixelSize: 11 }
                    Label { text: rotPanel.suggestions.length.toString(); color: "#ffaa00"; font.pixelSize: 28; font.bold: true }
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 100; radius: 10; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "System Health Score"; color: "#888"; font.pixelSize: 11 }
                    Label { text: "84 / 100"; color: "#0078d4"; font.pixelSize: 28; font.bold: true }
                }
            }
        }

        // ── Suggestions List ──────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Pruning Suggestions"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 5
                model: [
                    {path: "/Downloads/chrome_old_setup.exe", reason: "Obsolete Installer", confidence: 0.95, size: "124 MB"},
                    {path: "/Temp/cache_293.tmp", reason: "Temporary File", confidence: 1.0, size: "450 MB"},
                    {path: "/Documents/Project_Archive_2019.zip", reason: "Forgotten Data", confidence: 0.72, size: "2.4 GB"},
                    {path: "/Pictures/Blurry_001.jpg", reason: "AI: Blurry Photo", confidence: 0.85, size: "4.5 MB"}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 60; radius: 6
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        
                        Label { text: "🗑️"; font.pixelSize: 18 }
                        
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.path; color: "white"; font.pixelSize: 13; elide: Text.ElideMiddle; Layout.fillWidth: true }
                            RowLayout {
                                Label { text: modelData.reason; color: "#0078d4"; font.pixelSize: 11; font.bold: true }
                                Label { text: " • "; color: "#444" }
                                Label { text: "Confidence: " + Math.round(modelData.confidence * 100) + "%"; color: "#888"; font.pixelSize: 10 }
                            }
                        }
                        
                        Label { text: modelData.size; color: "#aaa"; font.pixelSize: 12; font.bold: true }
                        
                        Button {
                            text: "Remove"
                            background: Rectangle { color: "#3a1a1a"; radius: 4; border.color: "#ff4444" }
                            contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 11; font.bold: true }
                        }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── Controls Footer ───────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Button {
                text: "✨ Smart Purge (Remove >90% Confidence)"
                background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true }
            }
            Item { Layout.fillWidth: true }
            Button {
                text: "⚙️ Pruning Settings"
                flat: true; contentItem: Label { text: parent.text; color: "#888" }
            }
        }
    }
}
