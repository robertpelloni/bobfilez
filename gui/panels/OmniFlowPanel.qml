import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// OmniFlowPanel.qml — Visual Node-Based Automation Editor.
/// Drag, drop, and connect logic nodes to build complex file operations.
/// "The Unreal Engine Blueprints for File Management" - v4.4.0

Rectangle {
    id: flowPanel
    color: "#0f0f0f"; radius: 8

    property bool isExecuting: false
    property string activeWorkflow: "Auto-Vault Financials"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🌊 OmniFlow Automation"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Visual Logic Editor"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            ComboBox {
                model: ["Auto-Vault Financials", "Auto-WebM Conversion", "Compress Old Logs"]
                background: Rectangle { color: "#252525"; radius: 6; border.color: "#333" }
                contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 10 }
                implicitWidth: 200
            }
            
            Button {
                text: "▶ Run Workflow"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: flowPanel.isExecuting = true
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            // ── Node Palette (Left Sidebar) ──────────────────────────────────
            Rectangle {
                Layout.preferredWidth: 200; Layout.fillHeight: true
                color: "#161616"; radius: 6; border.color: "#333"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 5
                    
                    Label { text: "NODE PALETTE"; color: "#666"; font.bold: true; font.pixelSize: 10; Layout.bottomMargin: 5 }

                    // Triggers
                    Label { text: "TRIGGERS"; color: "#0078d4"; font.bold: true; font.pixelSize: 10 }
                    Button { text: "🕒 Schedule"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "👁️ Folder Watcher"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    
                    // Conditions
                    Label { text: "CONDITIONS"; color: "#ffaa00"; font.bold: true; font.pixelSize: 10; Layout.topMargin: 10 }
                    Button { text: "📄 Text Contains"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "🧠 AI Identifies"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "📊 Size > N"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }

                    // Actions
                    Label { text: "ACTIONS"; color: "#4caf50"; font.bold: true; font.pixelSize: 10; Layout.topMargin: 10 }
                    Button { text: "✂️ Move File"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "🗜️ Archive File"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "🔒 Encrypt in Vault"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "🔄 Convert Video"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    Button { text: "🔔 Send Notification"; flat: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    
                    Item { Layout.fillHeight: true }
                }
            }

            // ── Visual Node Canvas ───────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: "#111"; radius: 10; border.color: "#2a2a2a"
                clip: true

                // Background grid pattern
                Canvas {
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.strokeStyle = "#1a1a1a";
                        ctx.lineWidth = 1;
                        for (let x = 0; x < width; x += 40) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, height); ctx.stroke(); }
                        for (let y = 0; y < height; y += 40) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke(); }
                    }
                }

                // Bezier Connections (Simulated for visual demo)
                Canvas {
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.strokeStyle = "#0078d4"; ctx.lineWidth = 3;
                        
                        function drawBezier(x1, y1, x2, y2) {
                            ctx.beginPath(); ctx.moveTo(x1, y1);
                            ctx.bezierCurveTo(x1 + 100, y1, x2 - 100, y2, x2, y2);
                            ctx.stroke();
                        }
                        
                        drawBezier(220, 150, 350, 150); // Watcher -> PDF Filter
                        drawBezier(500, 150, 630, 250); // Filter -> OCR/Contains (True branch)
                        drawBezier(780, 250, 920, 150); // Contains -> Vault (True)
                        drawBezier(780, 250, 920, 350); // Contains -> Log (False)
                    }
                }

                // Node Component
                component FlowNode: Rectangle {
                    property string title: "Node"
                    property string typeColor: "#0078d4"
                    property var configText: ""
                    width: 150; height: 80; radius: 6; color: "#1a1a1a"; border.color: typeColor
                    
                    Rectangle { width: parent.width; height: 24; color: typeColor; radius: 6
                        Rectangle { width: parent.width; height: 12; anchors.bottom: parent.bottom; color: parent.color } // Flatten bottom corners
                        Label { anchors.centerIn: parent; text: parent.parent.title; color: "white"; font.bold: true; font.pixelSize: 11 }
                    }
                    
                    Label { anchors.centerIn: parent; text: parent.configText; color: "#aaa"; font.pixelSize: 10 }
                    
                    // Output pin
                    Rectangle { anchors.right: parent.right; anchors.rightMargin: -6; anchors.verticalCenter: parent.verticalCenter; width: 12; height: 12; radius: 6; color: "white"; border.color: "#333" }
                    // Input pin
                    Rectangle { anchors.left: parent.left; anchors.leftMargin: -6; anchors.verticalCenter: parent.verticalCenter; width: 12; height: 12; radius: 6; color: "white"; border.color: "#333" }
                    
                    // Simple drag logic
                    MouseArea { anchors.fill: parent; drag.target: parent }
                }

                // Instantiate Nodes
                FlowNode { x: 70; y: 110; title: "👁️ Watch Folder"; typeColor: "#0078d4"; configText: "~/Downloads" }
                FlowNode { x: 350; y: 110; title: "🔎 Filter"; typeColor: "#8b5cf6"; configText: "Extension == .pdf\n(Branch: True)" }
                FlowNode { x: 630; y: 210; title: "🤖 AI Text Scan"; typeColor: "#ffaa00"; configText: "Contains: 'Invoice'\n(Branch: True)" }
                FlowNode { x: 920; y: 110; title: "🔒 Encrypt Vault"; typeColor: "#4caf50"; configText: "Vault ID: Secure_1" }
                FlowNode { x: 920; y: 310; title: "🔔 Notify"; typeColor: "#4caf50"; configText: "Msg: Not an invoice" }

                // Execution overlay
                Rectangle {
                    visible: flowPanel.isExecuting
                    anchors.centerIn: parent; width: 250; height: 80; color: "#cc000000"; radius: 10; border.color: "#4caf50"
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 5
                        Label { text: "Workflow Running..."; color: "white"; font.bold: true }
                        ProgressBar { value: 0.7; Layout.fillWidth: true; background: Rectangle { color: "#333"; radius: 3; implicitHeight: 6 }; contentItem: Rectangle { color: "#4caf50"; radius: 3 } }
                    }
                }
            }
        }
    }
}
