import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// AutonomousMonitoringPanel.qml — Real-time performance dashboard.
/// Visualizes metrics for OmniFlow and Autonomous Sync protocols.

Rectangle {
    id: monitorPanel
    color: "#0f0f0f"; radius: 8

    property int totalWorkflows: fileModel.totalWorkflows
    property double avgDuration: fileModel.avgWorkflowDuration
    property int syncUploads: fileModel.totalSyncUploads
    property bool lastValidationOk: fileModel.lastValidationOk
    property int swarmPeers: fileModel.activeSwarmPeers
    property string lastCheckpoint: fileModel.lastCheckpointId

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        RowLayout {
            spacing: 15
            Label { text: "🛡️ Autonomous Health Monitor"; font.pixelSize: 24; font.bold: true; color: "white" }
            Rectangle { 
                width: 100; height: 24; color: monitorPanel.lastValidationOk ? "#1a472a" : "#471a1a"; radius: 12
                Label { anchors.centerIn: parent; text: monitorPanel.lastValidationOk ? "HEALTHY" : "CRITICAL"; color: monitorPanel.lastValidationOk ? "#4caf50" : "#f44336"; font.bold: true; font.pixelSize: 10 }
            }
            Item { Layout.fillWidth: true }
            Label { text: "v6.3.4 Autonomous Mode"; color: "#666"; font.pixelSize: 12 }
        }

        GridLayout {
            columns: 4; Layout.fillWidth: true; rowSpacing: 20; columnSpacing: 20

            // Stats Cards
            MetricCard { title: "Workflows Executed"; value: monitorPanel.totalWorkflows; icon: "🌊"; color: "#0078d4" }
            MetricCard { title: "Avg. Duration"; value: monitorPanel.avgDuration.toFixed(2) + "s"; icon: "⏱️"; color: "#8b5cf6" }
            MetricCard { title: "Sync Uploads"; value: monitorPanel.syncUploads; icon: "☁️"; color: "#03a9f4" }
            MetricCard { title: "Swarm Peers"; value: monitorPanel.swarmPeers; icon: "🐝"; color: "#ff9800" }
            MetricCard { title: "Last Checkpoint"; value: monitorPanel.lastCheckpoint || "None"; icon: "💾"; color: "#4caf50" }
            MetricCard { title: "System Uptime"; value: "4d 12h"; icon: "⚙️"; color: "#4caf50" }
        }

        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; color: "#161616"; radius: 10; border.color: "#333"
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 15; spacing: 10
                Label { text: "Real-time Processing Throughput"; color: "#888"; font.bold: true; font.pixelSize: 12 }
                
                // Real-time Throughput Visualization (Simulated for demo)
                Canvas {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.strokeStyle = "#4caf50"; ctx.lineWidth = 2;
                        ctx.beginPath();
                        ctx.moveTo(0, height * 0.8);
                        // Using avgDuration to scale the graph visually
                        var scale = Math.min(1.0, 1.0 / (monitorPanel.avgDuration + 0.1));
                        for(var i=0; i<width; i+=10) {
                            ctx.lineTo(i, height * (0.4 + (Math.random() * 0.2 * scale)));
                        }
                        ctx.stroke();
                        
                        // Fill area
                        ctx.lineTo(width, height); ctx.lineTo(0, height);
                        ctx.fillStyle = Qt.rgba(0.2, 0.6, 0.2, 0.1);
                        ctx.fill();
                    }
                }
            }
        }
    }

    component MetricCard: Rectangle {
        property string title; property var value; property string icon; property string color
        Layout.fillWidth: true; height: 100; color: "#1a1a1a"; radius: 8; border.color: "#333"
        ColumnLayout {
            anchors.centerIn: parent; spacing: 5
            RowLayout {
                spacing: 5; anchors.horizontalCenter: parent.horizontalCenter
                Label { text: icon; font.pixelSize: 16 }
                Label { text: title; color: "#aaa"; font.pixelSize: 12 }
            }
            Label { text: value; color: parent.parent.color; font.pixelSize: 22; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
        }
    }
}
