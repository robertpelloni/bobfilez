import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniClerkPanel.qml — Autonomous AI Secretary Dashboard.
/// A live feed of documents being ingested, OCR'd, analyzed, renamed,
/// and auto-filed into the correct folders without user intervention.

Rectangle {
    id: clerkPanel
    color: "#0f0f0f"; radius: 8

    property bool isListening: true
    property int documentsProcessed: 142
    property string activeFolder: "C:/Users/robert/Scans"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "💼 OmniClerk AI Secretary"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Autonomous Document Ingestion Pipeline"; color: "#888"; font.pixelSize: 14 }
            
            Rectangle { width: 80; height: 24; radius: 12; color: clerkPanel.isListening ? "#1a3a1a" : "#3a1a1a"
                border.color: clerkPanel.isListening ? "#4caf50" : "#ff4444"
                Label { anchors.centerIn: parent; text: clerkPanel.isListening ? "LISTENING" : "PAUSED"; color: clerkPanel.isListening ? "#4caf50" : "#ff4444"; font.pixelSize: 10; font.bold: true } }
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: clerkPanel.isListening ? "⏸ Pause Intake" : "▶ Start Intake"
                background: Rectangle { color: clerkPanel.isListening ? "#ff4444" : "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: clerkPanel.isListening = !clerkPanel.isListening
            }
        }

        // ── Configuration Row ─────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Rectangle {
                Layout.fillWidth: true; height: 40; radius: 6; color: "#161616"; border.color: "#333"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10; spacing: 10
                    Label { text: "📥 Inbox:"; color: "#888"; font.pixelSize: 11; font.bold: true }
                    TextInput { text: clerkPanel.activeFolder; color: "white"; font.pixelSize: 13; Layout.fillWidth: true }
                    Button { text: "Browse..."; flat: true; contentItem: Label { text: parent.text; color: "#0078d4" } }
                }
            }
            Rectangle {
                Layout.fillWidth: true; height: 40; radius: 6; color: "#161616"; border.color: "#333"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10; spacing: 10
                    Label { text: "📂 Auto-Filing Root:"; color: "#888"; font.pixelSize: 11; font.bold: true }
                    TextInput { text: "C:/Users/robert/Documents"; color: "white"; font.pixelSize: 13; Layout.fillWidth: true }
                    Button { text: "Browse..."; flat: true; contentItem: Label { text: parent.text; color: "#0078d4" } }
                }
            }
        }

        // ── Live Ingestion Feed (Middle) ──────────────────────────────────
        GroupBox {
            label: Label { text: "Recent Ingestion Receipts"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#111"; radius: 6; border.color: "#2a2a2a" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 10
                ScrollBar.vertical: ScrollBar {}
                model: [
                    {status: "✓", orig: "scan_001.pdf", filed: "/Finances/2024/AWS_Invoice_45.00.pdf", type: "Invoice", entities: "Vendor: AWS | Total: $45.00", time: "14:32:05"},
                    {status: "✓", orig: "IMG_2023.jpg", filed: "/Legal/Contracts/NDA_Bobfilez_Corp.pdf", type: "Contract", entities: "Party: Bobfilez Corp | Date: 2026-04-03", time: "14:28:44"},
                    {status: "⚠️", orig: "receipt_blurry.png", filed: "Pending User Review", type: "Unknown", entities: "Confidence: Low (< 40%)", time: "14:20:12"}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 80; radius: 8
                    color: itemHover.hovered ? "#252525" : "#1a1a1a"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        
                        // Status
                        Rectangle { width: 40; height: 40; radius: 20; color: modelData.status === "✓" ? "#1a3a1a" : "#3a2a1a"; border.color: modelData.status === "✓" ? "#4caf50" : "#ffaa00"
                            Label { anchors.centerIn: parent; text: modelData.status === "✓" ? "📄" : "❓"; font.pixelSize: 18 } }
                        
                        // Details
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            RowLayout {
                                Label { text: modelData.orig; color: "#888"; font.pixelSize: 12; font.strikeout: true }
                                Label { text: "➔"; color: "#0078d4" }
                                Label { text: modelData.filed; color: modelData.status === "✓" ? "white" : "#ffaa00"; font.pixelSize: 13; font.bold: true; elide: Text.ElideLeft; Layout.fillWidth: true }
                            }
                            
                            RowLayout {
                                spacing: 10
                                Rectangle { width: lblType.implicitWidth + 16; height: 20; radius: 10; color: "#0078d433"; border.color: "#0078d4"
                                    Label { id: lblType; anchors.centerIn: parent; text: modelData.type; color: "#0078d4"; font.pixelSize: 10; font.bold: true } }
                                Label { text: "AI Extracted: " + modelData.entities; color: "#ccc"; font.pixelSize: 10; font.family: "Consolas" }
                                Item { Layout.fillWidth: true }
                                Label { text: modelData.time; color: "#666"; font.pixelSize: 10 }
                            }
                        }

                        // Actions
                        Button {
                            text: modelData.status === "✓" ? "Reveal" : "Review"
                            background: Rectangle { color: modelData.status === "✓" ? "#22ffffff" : "#0078d4"; radius: 4 }
                            contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 11; font.bold: true }
                        }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── Stats Footer ──────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            Label { text: "Pipeline Stats:"; color: "#888"; font.pixelSize: 11 }
            Label { text: clerkPanel.documentsProcessed + " Docs Processed"; color: "#4caf50"; font.pixelSize: 12; font.bold: true }
            Label { text: "Avg OCR Time: 1.2s"; color: "#ccc"; font.pixelSize: 11 }
            Label { text: "Avg AI NER Time: 0.8s"; color: "#ccc"; font.pixelSize: 11 }
            Item { Layout.fillWidth: true }
            Button { text: "View Forensic Audit Trail"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
        }
    }
}
