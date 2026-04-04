import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// RecoveryPanel.qml — Forensic Data Recovery and Self-Healing UI.
/// Detect bit-rot, undelete files, and restore from Swarm/Cloud mirrors.
/// "The Data Resurrection" suite.

Rectangle {
    id: recoveryPanel
    color: "#0f0f0f"; radius: 8

    property bool isScrubbing: false
    property int corruptionCount: 2
    property var reports: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🩹 Data Recovery & Healing"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Bit-rot Protection"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: recoveryPanel.isScrubbing ? "⏳ Scrubbing Disk..." : "🔍 Start Data Scrub"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: recoveryPanel.isScrubbing = !recoveryPanel.isScrubbing
            }
        }

        // ── Health Dashboard ──────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            Rectangle {
                Layout.fillWidth: true; height: 100; radius: 10; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Files Verified"; color: "#888"; font.pixelSize: 11 }
                    Label { text: "124,532"; color: "white"; font.bold: true; font.pixelSize: 24 }
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 100; radius: 10; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Corruptions Detected"; color: "#888"; font.pixelSize: 11 }
                    Label { text: recoveryPanel.corruptionCount.toString(); color: recoveryPanel.corruptionCount > 0 ? "#ff4444" : "#4caf50"; font.bold: true; font.pixelSize: 24 }
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 100; radius: 10; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Redundancy Level"; color: "#888"; font.pixelSize: 11 }
                    Label { text: "3x Mirror"; color: "#0078d4"; font.bold: true; font.pixelSize: 24 }
                }
            }
        }

        // ── Corruption Reports List ────────────────────────────────────────
        GroupBox {
            label: Label { text: "Integrity Exceptions"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 8
                model: [
                    {path: "/Archive/wedding_01.raw", error: "Bit-rot: Checksum Mismatch", source: "AWS S3", canHeal: true},
                    {path: "/Work/Report_Final.pdf", error: "Silent Data Corruption", source: "Swarm: Laptop-X1", canHeal: true},
                    {path: "/Users/robert/key.txt", error: "File Missing (Deleted)", source: "Nexus Vault", canHeal: true}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 70; radius: 8
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        Label { text: "⚠️"; font.pixelSize: 24 }
                        
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.path; color: "white"; font.pixelSize: 13; elide: Text.ElideMiddle }
                            Label { text: modelData.error; color: "#ff4444"; font.pixelSize: 11; font.bold: true }
                        }
                        
                        ColumnLayout {
                            spacing: 4
                            Label { text: "Source: " + modelData.source; color: "#888"; font.pixelSize: 10 }
                            Button {
                                text: "Heal File"
                                background: Rectangle { color: "#1a3a1a"; border.color: "#4caf50"; radius: 4 }
                                contentItem: Label { text: parent.text; color: "#4caf50"; font.pixelSize: 10; font.bold: true }
                            }
                        }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── Controls Footer ───────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Button { text: "♻️ Undelete Browser"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
            Button { text: "📅 Scrub Schedule"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
            Item { Layout.fillWidth: true }
            Button {
                text: "✨ Heal All Corruptions"
                background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true }
            }
        }
    }
}
