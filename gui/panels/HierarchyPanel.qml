import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// HierarchyPanel.qml — AI-driven "Perfect Hierarchy" Generator.
/// Proposes a multi-level folder structure for messy directories.
/// Uses metadata clustering and CLIP categorization.

Rectangle {
    id: hierarchyPanel
    color: "#0f0f0f"; radius: 8

    property var proposedNodes: [] // [{name, filesCount, subfolders: []}]
    property bool isAnalyzing: false
    property real granularity: 0.5

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "📐 Auto-Hierarchy"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "AI Folder Structure Generator"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: hierarchyPanel.isAnalyzing ? "⏳ Thinking..." : "✨ Generate Structure"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: hierarchyPanel.isAnalyzing = !hierarchyPanel.isAnalyzing
            }
        }

        RowLayout {
            spacing: 20
            Label { text: "Granularity:"; color: "#888"; font.pixelSize: 11 }
            Slider { id: granSlider; from: 0; to: 1; value: 0.5; Layout.fillWidth: true; onValueChanged: hierarchyPanel.granularity = value }
            Label { text: Math.round(granSlider.value * 100) + "%"; color: "white"; width: 40 }
        }

        // ── Proposed Tree View ─────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Proposed Structure View"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 2
                model: [
                    {level: 0, name: "📁 Photos", count: 1240},
                    {level: 1, name: "📁 2024", count: 850},
                    {level: 2, name: "📁 January", count: 210},
                    {level: 3, name: "📁 Ski Trip", count: 45},
                    {level: 0, name: "📁 Documents", count: 450},
                    {level: 1, name: "📁 Invoices", count: 120},
                    {level: 1, name: "📁 Projects", count: 330}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 32; radius: 4
                    color: nodeHover.hovered ? "#22ffffff" : "transparent"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 15 + (modelData.level * 25); spacing: 10
                        Label { text: "▶"; color: "#555"; visible: modelData.level < 3 }
                        Label { text: modelData.name; color: "white"; font.pixelSize: 13 }
                        Item { Layout.fillWidth: true }
                        Label { text: modelData.count + " files"; color: "#666"; font.pixelSize: 11 }
                        Rectangle { width: 40; height: 16; radius: 8; color: "#0078d433"; border.color: "#0078d4"
                            Label { anchors.centerIn: parent; text: "94%"; color: "#0078d4"; font.pixelSize: 9; font.bold: true } }
                    }
                    HoverHandler { id: nodeHover }
                }
            }
        }

        // ── Controls Footer ───────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Button {
                text: "🚀 Commit Reorganization"
                background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true }
            }
            Button {
                text: "🔄 Re-Roll AI"
                flat: true; contentItem: Label { text: parent.text; color: "#888" }
            }
            Item { Layout.fillWidth: true }
            Label { text: "Target: D:/Organized_Library/"; color: "#666"; font.pixelSize: 11 }
            Button { text: "Change..."; flat: true; contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 11 } }
        }
    }
}
