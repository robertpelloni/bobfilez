import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/// Dashboard.qml — High-fidelity OmniShell Command Center.
/// Features system telemetry, Nexus task monitoring, and data cleanup metrics.

Rectangle {
    id: dashboard
    width: 400; height: 750
    color: "#e6111111"; radius: 12; border.color: "#33ffffff"
    
    layer.enabled: true
    layer.effect: DropShadow { radius: 15; samples: 20; color: "#aa000000" }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 20

        // ── User / System Header ──────────────────────────────────────────
        RowLayout {
            spacing: 12
            Rectangle { width: 40; height: 40; radius: 20; color: "#0078d4"
                Label { anchors.centerIn: parent; text: "R"; font.bold: true; color: "white" } }
            Column {
                Label { text: "Robert's Station"; color: "white"; font.bold: true; font.pixelSize: 14 }
                Label { text: "Online • Windows 11 Shell v3.2.0"; color: "#4caf50"; font.pixelSize: 10 }
            }
            Item { Layout.fillWidth: true }
            Button { text: "⚙️"; flat: true; contentItem: Label { text: parent.text; font.pixelSize: 16 } }
        }

        // ── Storage Overview ──────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Storage Health"; color: "#888"; font.bold: true; font.pixelSize: 11 }
            Layout.fillWidth: true
            background: Rectangle { color: "#11ffffff"; radius: 8 }
            
            ColumnLayout {
                anchors.fill: parent; spacing: 8
                RowLayout {
                    Label { text: "Local Disk (C:)"; color: "#ccc"; font.pixelSize: 11 }
                    Item { Layout.fillWidth: true }
                    Label { text: "450 GB free / 1 TB"; color: "#888"; font.pixelSize: 10 }
                }
                ProgressBar { value: 0.55; Layout.fillWidth: true; 
                    background: Rectangle { color: "#22ffffff"; radius: 4; implicitHeight: 6 }
                    contentItem: Rectangle { color: "#0078d4"; radius: 4 } }
                
                RowLayout {
                    spacing: 15; Layout.topMargin: 5
                    Column { Label { text: "124 GB"; color: "#38bdf8"; font.bold: true; font.pixelSize: 18 }; Label { text: "Cleaned"; color: "#666"; font.pixelSize: 9 } }
                    Column { Label { text: "8,432"; color: "#fbbf24"; font.bold: true; font.pixelSize: 18 }; Label { text: "Files Organized"; color: "#666"; font.pixelSize: 9 } }
                    Column { Label { text: "94%"; color: "#34d399"; font.bold: true; font.pixelSize: 18 }; Label { text: "Efficiency"; color: "#666"; font.pixelSize: 9 } }
                }
            }
        }

        // ── Nexus Task Monitor ────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Nexus Active Tasks"; color: "#888"; font.bold: true; font.pixelSize: 11 }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#11ffffff"; radius: 8 }

            ListView {
                anchors.fill: parent; clip: true; spacing: 8
                model: [
                    {name: "Shadow Sorter", sub: "Indexing /Downloads", p: 0.65, prio: "Idle"},
                    {name: "Video Transcode", sub: "vacation.mp4 → WebM", p: 0.12, prio: "Low"},
                    {name: "PII Sentinel", sub: "Scanning /Documents", p: 0.94, prio: "Normal"}
                ]
                delegate: ColumnLayout {
                    width: parent.width; spacing: 4
                    RowLayout {
                        Label { text: modelData.name; color: "white"; font.pixelSize: 11; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Label { text: modelData.prio; color: "#0078d4"; font.pixelSize: 9 }
                    }
                    ProgressBar { value: modelData.p; Layout.fillWidth: true;
                        background: Rectangle { color: "#22ffffff"; radius: 3; implicitHeight: 4 }
                        contentItem: Rectangle { color: "#0078d4"; radius: 3 } }
                    Label { text: modelData.sub; color: "#666"; font.pixelSize: 9 }
                }
            }
        }

        // ── System Performance ────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 8; color: "#11ffffff"
                Column { anchors.centerIn: parent; spacing: 2
                    Label { text: "CPU"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                    Label { text: "14%"; color: "white"; font.bold: true; font.pixelSize: 20; anchors.horizontalCenter: parent.horizontalCenter }
                    // Mini graph line here
                }
            }
            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 8; color: "#11ffffff"
                Column { anchors.centerIn: parent; spacing: 2
                    Label { text: "I/O"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                    Label { text: "42 MB/s"; color: "#4caf50"; font.bold: true; font.pixelSize: 16; anchors.horizontalCenter: parent.horizontalCenter }
                    Label { text: "Disk 0 (SSD)"; color: "#666"; font.pixelSize: 8; anchors.horizontalCenter: parent.horizontalCenter }
                }
            }
        }

        // ── Quick Actions ─────────────────────────────────────────────────
        Flow {
            Layout.fillWidth: true; spacing: 8
            component QuickAction: Button {
                property string icon: ""
                padding: 8
                background: Rectangle { color: hovered ? "#33ffffff" : "#11ffffff"; radius: 6 }
                contentItem: RowLayout { spacing: 6
                    Label { text: parent.icon; font.pixelSize: 14 }
                    Label { text: parent.text; color: "white"; font.pixelSize: 11 } }
            }
            QuickAction { text: "Clean Temp"; icon: "🧹" }
            QuickAction { text: "Verify DB"; icon: "🛡️" }
            QuickAction { text: "Sync All"; icon: "🔄" }
            QuickAction { text: "Snapshot"; icon: "📸" }
        }
    }
}
