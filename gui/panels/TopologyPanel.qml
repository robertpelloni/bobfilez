import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Omni.Viz 1.0

/// TopologyPanel.qml — Visual "Data Topology" Map (Tree-map).
/// Visualizes disk usage using nested rectangles where area = file size.

Rectangle {
    id: topologyPanel
    color: "#0a0a0a"; radius: 8

    property string rootPath: "C:/"
    property var selectedItem: null

    TreemapModel { id: treemapModel; rootPath: topologyPanel.rootPath }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🌲 Data Topology"; font.pixelSize: 20; font.bold: true; color: "white" }
            
            Rectangle {
                Layout.fillWidth: true; height: 32; radius: 6; color: "#1a1a1a"; border.color: "#333"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10
                    Label { text: "📁"; color: "#888" }
                    TextInput { 
                        id: pathInput; text: topologyPanel.rootPath; color: "white"; Layout.fillWidth: true; font.pixelSize: 12
                        onAccepted: topologyPanel.rootPath = text
                    }
                }
            }

            Button {
                text: treemapModel.isBusy ? "⏳ Scanning..." : "▶ Analyze Disk"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: treemapModel.refresh()
            }
        }

        // ── Main Map Area ──────────────────────────────────────────────────
        Item {
            Layout.fillWidth: true; Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent; color: "#050505"; border.color: "#222"
                clip: true

                Repeater {
                    model: treemapModel
                    delegate: Rectangle {
                        x: rx * parent.width; y: ry * parent.height
                        width: rw * parent.width; height: rh * parent.height
                        color: type === "Video" ? "#3a2a1a" : type === "Image" ? "#1a3a1a" : "#2d3a4a"
                        border.color: Qt.lighter(color)
                        opacity: hoverNode.hovered ? 1.0 : 0.7
                        
                        Label {
                            anchors.centerIn: parent
                            visible: width > 40 && height > 20
                            text: name; color: "white"; font.pixelSize: 10; elide: Text.ElideRight; width: parent.width - 4
                        }

                        HoverHandler { id: hoverNode }
                        ToolTip.visible: hoverNode.hovered; ToolTip.text: path + "\nSize: " + (size / (1024*1024)).toFixed(1) + " MB"
                    }
                }
            }
        }

        // ── Details Footer ────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 100; color: "#161616"; radius: 6; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.margins: 15; spacing: 20

                // Large Type Breakdown
                ColumnLayout {
                    Label { text: "Type Breakdown"; color: "#888"; font.pixelSize: 11; font.bold: true }
                    Row {
                        spacing: 4
                        Rectangle { width: 40; height: 10; color: "#4a9eff" }; Label { text: "System"; color: "#ccc"; font.pixelSize: 10 }
                        Rectangle { width: 40; height: 10; color: "#ffaa00" }; Label { text: "Video"; color: "#ccc"; font.pixelSize: 10 }
                        Rectangle { width: 40; height: 10; color: "#4caf50" }; Label { text: "Image"; color: "#ccc"; font.pixelSize: 10 }
                        Rectangle { width: 40; height: 10; color: "#8b5cf6" }; Label { text: "Apps"; color: "#ccc"; font.pixelSize: 10 }
                    }
                }

                Item { Layout.fillWidth: true }

                // Selection Info
                ColumnLayout {
                    Layout.preferredWidth: 300
                    Label { text: "Selected: /System/Library/Caches/..."; color: "white"; font.pixelSize: 12; elide: Text.ElideLeft; Layout.fillWidth: true }
                    Label { text: "Size: 1.2 GB  |  Files: 4,210  |  Modified: 2h ago"; color: "#888"; font.pixelSize: 11 }
                    RowLayout {
                        Button { text: "🗑 Delete"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 11 } }
                        Button { text: "📂 Open"; flat: true; contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 11 } }
                        Button { text: "✨ Clean"; flat: true; contentItem: Label { text: parent.text; color: "#4caf50"; font.pixelSize: 11 } }
                    }
                }
            }
        }
    }
}
