import QtQuick 2.15
import QtQuick.Layouts 1.15

/// Dashboard.qml — High-fidelity OmniShell Command Center.
/// Features system telemetry, Nexus task monitoring, and data cleanup metrics.

Rectangle {
    id: dashboard
    width: 400; height: 750
    color: "#e6111111"; radius: 12; border.color: "#33ffffff"

    component SlimProgressBar: Item {
        id: progressRoot
        property real value: 0.0
        property color fillColor: "#0078d4"
        property int barHeight: 6
        implicitWidth: 100
        implicitHeight: barHeight

        Rectangle {
            anchors.fill: parent
            radius: progressRoot.barHeight / 2
            color: "#22ffffff"
        }

        Rectangle {
            width: parent.width * Math.max(0, Math.min(1, progressRoot.value))
            height: parent.height
            radius: progressRoot.barHeight / 2
            color: progressRoot.fillColor
        }
    }

    component QuickActionChip: Rectangle {
        id: chipRoot
        property string icon: ""
        property string label: ""

        radius: 6
        height: 34
        width: chipRow.implicitWidth + 16
        color: chipHover.hovered ? "#33ffffff" : "#11ffffff"

        Row {
            id: chipRow
            anchors.centerIn: parent
            spacing: 6

            Text {
                text: chipRoot.icon
                font.pixelSize: 14
                color: "white"
            }

            Text {
                text: chipRoot.label
                color: "white"
                font.pixelSize: 11
            }
        }

        HoverHandler { id: chipHover }
        MouseArea { anchors.fill: parent }
    }
    
    Rectangle {
        anchors.fill: parent
        anchors.margins: -2
        radius: 14
        color: "transparent"
        border.color: "#22000000"
        z: -1
    }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 20

        // ── User / System Header ──────────────────────────────────────────
        RowLayout {
            spacing: 12

            Rectangle {
                width: 40; height: 40; radius: 20; color: "#0078d4"

                Text {
                    anchors.centerIn: parent
                    text: "R"
                    font.bold: true
                    color: "white"
                }
            }

            Column {
                Text { text: "Robert's Station"; color: "white"; font.bold: true; font.pixelSize: 14 }
                Text { text: "Online • Windows 11 Shell v3.2.0"; color: "#4caf50"; font.pixelSize: 10 }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 32; height: 32; radius: 6
                color: settingsHover.hovered ? "#22ffffff" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "⚙️"
                    font.pixelSize: 16
                }

                HoverHandler { id: settingsHover }
                MouseArea { anchors.fill: parent }
            }
        }

        // ── Storage Overview ──────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: "Storage Health"
                color: "#888"
                font.bold: true
                font.pixelSize: 11
            }

            Rectangle {
                Layout.fillWidth: true
                radius: 8
                color: "#11ffffff"
                implicitHeight: 112

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    RowLayout {
                        Text { text: "Local Disk (C:)"; color: "#ccc"; font.pixelSize: 11 }
                        Item { Layout.fillWidth: true }
                        Text { text: "450 GB free / 1 TB"; color: "#888"; font.pixelSize: 10 }
                    }

                    SlimProgressBar {
                        value: 0.55
                        Layout.fillWidth: true
                        fillColor: "#0078d4"
                        barHeight: 6
                    }
                    
                    RowLayout {
                        spacing: 15
                        Layout.topMargin: 5

                        Column {
                            Text { text: "124 GB"; color: "#38bdf8"; font.bold: true; font.pixelSize: 18 }
                            Text { text: "Cleaned"; color: "#666"; font.pixelSize: 9 }
                        }

                        Column {
                            Text { text: "8,432"; color: "#fbbf24"; font.bold: true; font.pixelSize: 18 }
                            Text { text: "Files Organized"; color: "#666"; font.pixelSize: 9 }
                        }

                        Column {
                            Text { text: "94%"; color: "#34d399"; font.bold: true; font.pixelSize: 18 }
                            Text { text: "Efficiency"; color: "#666"; font.pixelSize: 9 }
                        }
                    }
                }
            }
        }

        // ── Nexus Task Monitor ────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6

            Text {
                text: "Nexus Active Tasks"
                color: "#888"
                font.bold: true
                font.pixelSize: 11
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 8
                color: "#11ffffff"

                ListView {
                    anchors.fill: parent
                    anchors.margins: 12
                    clip: true
                    spacing: 8
                    model: [
                        {name: "Shadow Sorter", sub: "Indexing /Downloads", p: 0.65, prio: "Idle"},
                        {name: "Video Transcode", sub: "vacation.mp4 → WebM", p: 0.12, prio: "Low"},
                        {name: "PII Sentinel", sub: "Scanning /Documents", p: 0.94, prio: "Normal"}
                    ]

                    delegate: ColumnLayout {
                        width: ListView.view.width
                        spacing: 4

                        RowLayout {
                            Text { text: modelData.name; color: "white"; font.pixelSize: 11; font.bold: true }
                            Item { Layout.fillWidth: true }
                            Text { text: modelData.prio; color: "#0078d4"; font.pixelSize: 9 }
                        }

                        SlimProgressBar {
                            value: modelData.p
                            Layout.fillWidth: true
                            fillColor: "#0078d4"
                            barHeight: 4
                        }

                        Text { text: modelData.sub; color: "#666"; font.pixelSize: 9 }
                    }
                }
            }
        }

        // ── System Performance ────────────────────────────────────────────
        RowLayout {
            spacing: 15

            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 8; color: "#11ffffff"

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    Text { text: "CPU"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: "14%"; color: "white"; font.bold: true; font.pixelSize: 20; anchors.horizontalCenter: parent.horizontalCenter }
                    // Mini graph line here
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 8; color: "#11ffffff"

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    Text { text: "I/O"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: "42 MB/s"; color: "#4caf50"; font.bold: true; font.pixelSize: 16; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: "Disk 0 (SSD)"; color: "#666"; font.pixelSize: 8; anchors.horizontalCenter: parent.horizontalCenter }
                }
            }
        }

        // ── Quick Actions ─────────────────────────────────────────────────
        Flow {
            Layout.fillWidth: true
            spacing: 8

            QuickActionChip { label: "Clean Temp"; icon: "🧹" }
            QuickActionChip { label: "Verify DB"; icon: "🛡️" }
            QuickActionChip { label: "Sync All"; icon: "🔄" }
            QuickActionChip { label: "Snapshot"; icon: "📸" }
        }
    }
}
