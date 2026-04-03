import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// AssetManagerPanel.qml — Universal Media Asset Manager (MAM).
/// Combines preview, transcoding, tagging, and organization for pro assets.
/// Parity with specialized asset management suites.

Rectangle {
    id: assetPanel
    color: "#0f0f0f"; radius: 8

    property var selectedFiles: []
    property bool isProcessing: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "📦 Asset Manager"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Pro Media Control Center"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: "🚀 Batch Process"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: batchMenu.open()
                Menu {
                    id: batchMenu
                    MenuItem { text: "🏷️ Auto-Tag with AI" }
                    MenuItem { text: "🔄 Transcode to WebM" }
                    MenuItem { text: "🖼️ Generate Proxies" }
                    MenuItem { text: "🗜️ Archive to Cold Storage" }
                }
            }
        }

        // ── Asset Preview & Metadata ──────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 20

            // Left: Large Preview
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; color: "black"; radius: 10
                Image {
                    anchors.fill: parent; anchors.margins: 10
                    source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectFit
                }
                // Playback Overlay (for Video)
                Rectangle { anchors.centerIn: parent; width: 60; height: 60; radius: 30; color: "#aa000000"
                    Label { anchors.centerIn: parent; text: "▶"; color: "white"; font.pixelSize: 24 } }
            }

            // Right: Metadata & AI Tags
            Rectangle {
                width: 300; Layout.fillHeight: true; color: "#161616"; radius: 10; border.color: "#333"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 15; spacing: 10
                    
                    Label { text: "METADATA"; color: "#888"; font.bold: true; font.pixelSize: 11 }
                    GridLayout { columns: 2; rowSpacing: 5
                        Label { text: "Codec:"; color: "#666" }; Label { text: "H.264 / AAC"; color: "#ccc" }
                        Label { text: "Duration:"; color: "#666" }; Label { text: "04:32"; color: "#ccc" }
                        Label { text: "Bitrate:"; color: "#666" }; Label { text: "12.4 Mbps"; color: "#ccc" }
                    }

                    Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                    Label { text: "AI SUGGESTED TAGS"; color: "#888"; font.bold: true; font.pixelSize: 11 }
                    Flow {
                        Layout.fillWidth: true; spacing: 5
                        Repeater {
                            model: ["Outdoor", "Sunset", "Beach", "Cinematic", "Drone"]
                            Rectangle {
                                width: tagL.implicitWidth + 20; height: 26; radius: 13
                                color: "#0078d422"; border.color: "#0078d4"
                                Label { id: tagL; anchors.centerIn: parent; text: modelData; color: "#0078d4"; font.pixelSize: 11 }
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                    
                    Button { text: "📄 Open in Editor"; Layout.fillWidth: true }
                    Button { text: "📂 Reveal in Explorer"; Layout.fillWidth: true }
                }
            }
        }

        // ── Asset Library Grid ─────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Media Pool"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; height: 250
            background: Rectangle { color: "#111"; radius: 6; border.color: "#333" }
            padding: 10

            GridView {
                anchors.fill: parent; clip: true; cellWidth: 160; cellHeight: 120
                model: 24
                delegate: Rectangle {
                    width: 150; height: 110; color: index === 0 ? "#0078d433" : "#1a1a1a"; radius: 6
                    border.color: index === 0 ? "#0078d4" : "#333"
                    Image { anchors.fill: parent; anchors.margins: 4; source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectCrop }
                    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 20; color: "#cc000000"; radius: 4
                        Label { anchors.centerIn: parent; text: "00:45"; color: "white"; font.pixelSize: 9 } }
                }
            }
        }
    }
}
