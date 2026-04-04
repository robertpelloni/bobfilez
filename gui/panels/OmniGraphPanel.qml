import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniGraphPanel.qml — Visual Force-Directed Node Graph for Files.
/// Visualizes semantic relationships between documents, photos, people, and places.
/// "The Death of the Folder" — Navigate your data naturally.

Rectangle {
    id: graphPanel
    color: "#0a0a0a"; radius: 8

    property bool isGenerating: false
    property string activeNodeInfo: "Hover over a node to see its semantic links."

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🕸️ OmniGraph"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Semantic File Relationships"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: graphPanel.isGenerating ? "⏳ Analyzing Concepts..." : "🔍 Rebuild Knowledge Graph"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: graphPanel.isGenerating = !graphPanel.isGenerating
            }
        }

        // ── Main Graph Canvas ─────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#111"; radius: 10; border.color: "#333"
            clip: true

            // A mocked force-directed graph UI (In production, this could be D3.js in WebEngine 
            // or an actual QML force layout, but we'll use a hardcoded layout for the shell preview)

            // Links (Edges)
            Canvas {
                id: graphCanvas
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.lineWidth = 2;
                    ctx.strokeStyle = "#444";
                    
                    function drawLine(x1, y1, x2, y2) {
                        ctx.beginPath();
                        ctx.moveTo(x1, y1);
                        ctx.lineTo(x2, y2);
                        ctx.stroke();
                    }

                    // Draw mocked connections
                    drawLine(250, 150, 450, 250); // Alice -> Paris
                    drawLine(450, 250, 600, 180); // Paris -> img_4882
                    drawLine(450, 250, 650, 350); // Paris -> eiffel_video
                    drawLine(250, 150, 600, 180); // Alice -> img_4882
                    drawLine(250, 150, 650, 350); // Alice -> eiffel_video
                    
                    ctx.strokeStyle = "#8b5cf688"; // Purple connection for Concepts
                    drawLine(450, 250, 350, 450); // Paris -> Vacation
                    drawLine(350, 450, 150, 380); // Vacation -> trip_budget
                    drawLine(350, 450, 200, 500); // Vacation -> hotel_receipt
                }
            }

            // Nodes
            component GraphNode: Rectangle {
                property string title: ""
                property string typeColor: "#0078d4"
                property real nodeSize: 50
                property string icon: "📄"
                
                width: nodeSize; height: nodeSize; radius: nodeSize/2
                color: hoverArea.hovered ? Qt.lighter(typeColor, 1.2) : typeColor
                border.color: "white"; border.width: 2
                
                Label { anchors.centerIn: parent; text: parent.icon; font.pixelSize: parent.nodeSize/2.5 }
                Label {
                    text: parent.title; color: "white"; font.pixelSize: 11; font.bold: true
                    anchors.top: parent.bottom; anchors.topMargin: 5; anchors.horizontalCenter: parent.horizontalCenter
                    style: Text.Outline; styleColor: "black"
                }

                HoverHandler {
                    id: hoverArea
                    onHoveredChanged: {
                        if (hovered) graphPanel.activeNodeInfo = "Node: " + parent.title + " | Connections: " + Math.floor(Math.random()*15 + 2) + " linked files."
                    }
                }
                
                // Entrance animation
                SequentialAnimation on scale {
                    NumberAnimation { from: 0.0; to: 1.2; duration: 400; easing.type: Easing.OutBack }
                    NumberAnimation { from: 1.2; to: 1.0; duration: 200 }
                }
            }

            // People
            GraphNode { x: 250-nodeSize/2; y: 150-nodeSize/2; title: "Alice"; typeColor: "#ffaa00"; nodeSize: 80; icon: "👤" }
            
            // Locations
            GraphNode { x: 450-nodeSize/2; y: 250-nodeSize/2; title: "Paris, France"; typeColor: "#4caf50"; nodeSize: 90; icon: "📍" }

            // Concepts
            GraphNode { x: 350-nodeSize/2; y: 450-nodeSize/2; title: "Vacation"; typeColor: "#8b5cf6"; nodeSize: 70; icon: "💡" }

            // Files
            GraphNode { x: 600-nodeSize/2; y: 180-nodeSize/2; title: "img_4882.jpg"; typeColor: "#333"; nodeSize: 50; icon: "📷" }
            GraphNode { x: 650-nodeSize/2; y: 350-nodeSize/2; title: "eiffel_vid.mp4"; typeColor: "#333"; nodeSize: 60; icon: "🎬" }
            GraphNode { x: 150-nodeSize/2; y: 380-nodeSize/2; title: "budget.xlsx"; typeColor: "#333"; nodeSize: 45; icon: "📊" }
            GraphNode { x: 200-nodeSize/2; y: 500-nodeSize/2; title: "hotel_rcpt.pdf"; typeColor: "#333"; nodeSize: 45; icon: "📄" }

            // Hover Info Display
            Rectangle {
                anchors.left: parent.left; anchors.bottom: parent.bottom; anchors.margins: 15
                width: 400; height: 30; color: "#aa000000"; radius: 6
                Label { anchors.centerIn: parent; text: graphPanel.activeNodeInfo; color: "#aaa"; font.pixelSize: 11; font.family: "Consolas" }
            }
        }
    }
}
