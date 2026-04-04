import QtQuick 2.15
import QtQuick.Layouts 1.15

/// SwarmPanel.qml — Multi-device Sync and Distributed Nexus Manager.
/// Control how multiple bobfilez nodes communicate and share indexes.
/// "The Distributed Data Custodian" v4.0.

Rectangle {
    id: swarmPanel
    color: "#0f0f0f"
    radius: 8

    property bool isDiscovering: true
    property var activeNodes: [] // [{id, host, ip, status}]

    component PanelButton: Rectangle {
        id: panelButton
        property alias text: buttonText.text
        property color textColor: "white"
        property color fillColor: "#0078d4"
        property color borderLineColor: "transparent"
        property bool flat: false
        signal clicked

        radius: 6
        height: 32
        color: flat ? (buttonHover.hovered ? "#22ffffff" : "transparent") : fillColor
        border.color: flat ? "transparent" : borderLineColor
        implicitWidth: buttonText.implicitWidth + 24

        Text {
            id: buttonText
            anchors.centerIn: parent
            color: panelButton.textColor
            font.bold: true
            font.pixelSize: 12
        }

        HoverHandler { id: buttonHover }
        MouseArea {
            anchors.fill: parent
            onClicked: panelButton.clicked()
        }
    }

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
            color: "#1a1a1a"
        }

        Rectangle {
            width: parent.width * Math.max(0, Math.min(1, progressRoot.value))
            height: parent.height
            radius: progressRoot.barHeight / 2
            color: progressRoot.fillColor
        }
    }

    function statusColor(status) {
        if (status === "Online") {
            return "#4caf50"
        }
        return "#ffaa00"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15

            Text {
                text: "🛸 Data Swarm"
                font.pixelSize: 22
                font.bold: true
                color: "white"
            }

            Text {
                text: "Distributed Nexus Control"
                color: "#888"
                font.pixelSize: 14
            }

            Item { Layout.fillWidth: true }

            PanelButton {
                text: "🔍 Refresh Peers"
                fillColor: "#0078d4"
                onClicked: swarmPanel.isDiscovering = true
            }
        }

        // ── World Map (Visualization) ─────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 150
            radius: 10
            color: "#0d0d0d"
            border.color: "#333"
            clip: true

            Text {
                anchors.centerIn: parent
                text: "🌐 Peer Map Visualization (Active)"
                color: "#444"
                font.pixelSize: 14
            }
            
            // Pulse effects for active nodes
            Rectangle {
                x: 100
                y: 40
                width: 8
                height: 8
                radius: 4
                color: "#4caf50"

                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    NumberAnimation { from: 0.2; to: 1.0; duration: 1000 }
                }
            }

            Rectangle {
                x: 300
                y: 80
                width: 8
                height: 8
                radius: 4
                color: "#0078d4"
            }
        }

        // ── Active Peers List ─────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: "Detected Nodes"
                color: "#aaa"
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 6
                color: "#161616"
                border.color: "#333"

                ListView {
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    spacing: 8
                    model: [
                        {id: "node-01", host: "ROBERT-PC", ip: "192.168.1.12", role: "Main Station", status: "Online"},
                        {id: "node-02", host: "LAPTOP-X1", ip: "192.168.1.45", role: "Field Unit", status: "Syncing..."},
                        {id: "node-03", host: "NAS-PRO", ip: "192.168.1.50", role: "Storage Node", status: "Online"},
                        {id: "node-04", host: "REMOTE-VM", ip: "45.32.10.124", role: "Cloud Relay", status: "Idle"}
                    ]
                    
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 60
                        radius: 8
                        color: itemHover.hovered ? "#252525" : "#1e1e1e"
                        border.color: "#333"
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 15
                            
                            Text {
                                text: "💻"
                                font.pixelSize: 24
                                color: "white"
                            }
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: modelData.host
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: 13
                                }

                                Text {
                                    text: modelData.role + " • " + modelData.ip
                                    color: "#888"
                                    font.pixelSize: 10
                                }
                            }
                            
                            ColumnLayout {
                                spacing: 2
                                Layout.preferredWidth: 80

                                Text {
                                    text: modelData.status
                                    color: swarmPanel.statusColor(modelData.status)
                                    font.pixelSize: 10
                                    font.bold: true
                                    horizontalAlignment: Text.AlignRight
                                }

                                Text {
                                    text: "Ping: 12ms"
                                    color: "#666"
                                    font.pixelSize: 9
                                    horizontalAlignment: Text.AlignRight
                                }
                            }
                            
                            PanelButton {
                                text: "Sync"
                                height: 28
                                fillColor: "#0078d433"
                                borderLineColor: "#0078d4"
                                textColor: "#0078d4"
                            }
                            
                            PanelButton {
                                text: "⚙️"
                                height: 28
                                flat: true
                                textColor: "#666"
                            }
                        }

                        HoverHandler { id: itemHover }
                    }
                }
            }
        }

        // ── Swarm Stats ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20

            Text {
                text: "Swarm Throughput:"
                color: "#888"
                font.pixelSize: 11
            }

            SlimProgressBar {
                value: 0.35
                Layout.fillWidth: true
                fillColor: "#0078d4"
                barHeight: 6
            }

            Text {
                text: "452 MB/s Total"
                color: "#4caf50"
                font.pixelSize: 11
                font.bold: true
            }
        }
    }
}
