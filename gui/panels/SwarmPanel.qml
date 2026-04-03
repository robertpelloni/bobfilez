import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// SwarmPanel.qml — Multi-device Sync and Distributed Nexus Manager.
/// Control how multiple bobfilez nodes communicate and share indexes.
/// "The Distributed Data Custodian" v4.0.

Rectangle {
    id: swarmPanel
    color: "#0f0f0f"; radius: 8

    property bool isDiscovering: true
    property var activeNodes: [] // [{id, host, ip, status}]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🛸 Data Swarm"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Distributed Nexus Control"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: "🔍 Refresh Peers"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: swarmPanel.isDiscovering = true
            }
        }

        // ── World Map (Visualization) ─────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 150; radius: 10; color: "#0d0d0d"; border.color: "#333"
            clip: true
            Label { anchors.centerIn: parent; text: "🌐 Peer Map Visualization (Active)"; color: "#444"; font.pixelSize: 14 }
            
            // Pulse effects for active nodes
            Rectangle { x: 100; y: 40; width: 8; height: 8; radius: 4; color: "#4caf50"
                SequentialAnimation on opacity { loops: Animation.Infinite; NumberAnimation { from: 0.2; to: 1.0; duration: 1000 } } }
            Rectangle { x: 300; y: 80; width: 8; height: 8; radius: 4; color: "#0078d4" }
        }

        // ── Active Peers List ─────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Detected Nodes"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 8
                model: [
                    {id: "node-01", host: "ROBERT-PC", ip: "192.168.1.12", role: "Main Station", status: "Online"},
                    {id: "node-02", host: "LAPTOP-X1", ip: "192.168.1.45", role: "Field Unit", status: "Syncing..."},
                    {id: "node-03", host: "NAS-PRO", ip: "192.168.1.50", role: "Storage Node", status: "Online"},
                    {id: "node-04", host: "REMOTE-VM", ip: "45.32.10.124", role: "Cloud Relay", status: "Idle"}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 60; radius: 8
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        
                        Label { text: "💻"; font.pixelSize: 24 }
                        
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.host; color: "white"; font.bold: true; font.pixelSize: 13 }
                            Label { text: modelData.role + " • " + modelData.ip; color: "#888"; font.pixelSize: 10 }
                        }
                        
                        ColumnLayout {
                            spacing: 2; Layout.preferredWidth: 80
                            Label { text: modelData.status; color: modelData.status === "Online" ? "#4caf50" : "#ffaa00"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignRight }
                            Label { text: "Ping: 12ms"; color: "#666"; font.pixelSize: 9; horizontalAlignment: Text.AlignRight }
                        }
                        
                        Button { text: "Sync"; background: Rectangle { color: "#0078d433"; border.color: "#0078d4"; radius: 4 }
                            contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 10; font.bold: true } }
                        
                        Button { text: "⚙️"; flat: true; contentItem: Label { text: parent.text; color: "#666" } }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── Swarm Stats ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            Label { text: "Swarm Throughput:"; color: "#888"; font.pixelSize: 11 }
            ProgressBar { value: 0.35; Layout.fillWidth: true; background: Rectangle { color: "#1a1a1a"; radius: 4; implicitHeight: 6 } }
            Label { text: "452 MB/s Total"; color: "#4caf50"; font.pixelSize: 11; font.bold: true }
        }
    }
}
