import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniPeekOverlay.qml — Universal Instant Preview Engine for bobfilez.
/// Pops up over the Explorer when pressing Spacebar to render 3D models,
/// videos, databases, archives, and binaries using the OmniPeek C++ engine.

Rectangle {
    id: peekOverlay
    anchors.fill: parent
    color: "#aa000000" // Dim the background
    z: 9999

    property string filePath: "C:/Projects/Engine/main_character.fbx"
    property string peekType: "Model3D" // "Video", "DatabaseSchema", "BinaryAssembly", "ArchiveTree", "TextCode", "Image"
    property string contentPayload: ""
    property string metaDataText: "Vertices: 24,532 | Materials: 3"
    property real loadTimeMs: 1.2

    MouseArea {
        anchors.fill: parent
        onClicked: peekOverlay.visible = false // Click outside to close
    }

    Rectangle {
        width: 800; height: 600
        anchors.centerIn: parent
        color: "#161616"; radius: 12
        border.color: "#333"; border.width: 1

        MouseArea { anchors.fill: parent } // Block clicks from falling through

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 10; spacing: 0

            // ── Top Bar ────────────────────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true; height: 40; color: "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
                    Label { text: peekOverlay.peekType === "Model3D" ? "🧊" : 
                                  peekOverlay.peekType === "Video" ? "🎬" :
                                  peekOverlay.peekType === "DatabaseSchema" ? "🗄️" :
                                  peekOverlay.peekType === "BinaryAssembly" ? "⚙️" :
                                  peekOverlay.peekType === "ArchiveTree" ? "📦" : "📄"; font.pixelSize: 18 }
                    Label { text: peekOverlay.filePath.split("/").pop(); color: "white"; font.pixelSize: 14; font.bold: true; elide: Text.ElideMiddle; Layout.fillWidth: true }
                    Label { text: "⚡ " + peekOverlay.loadTimeMs.toFixed(1) + " ms"; color: "#0078d4"; font.pixelSize: 11; font.bold: true }
                    Button { text: "Open Externally"; flat: true; contentItem: Label { text: parent.text; color: "#aaa"; font.pixelSize: 11 } }
                    Button { text: "✕"; flat: true; onClicked: peekOverlay.visible = false; contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 16 } }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }

            // ── Dynamic Content Area ───────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; color: "#0a0a0a"; radius: 6
                clip: true

                // Mock 3D Model Renderer (raylib integration)
                Rectangle {
                    visible: peekOverlay.peekType === "Model3D"
                    anchors.fill: parent; color: "#1e1e1e"
                    Label { anchors.centerIn: parent; text: "🧊 3D Canvas rendering (Interactive via raylib)"; color: "#888"; font.pixelSize: 14 }
                    // Rotation controls
                    Row { anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottomMargin: 15; spacing: 10
                        Button { text: "↺"; flat: true }
                        Button { text: "Reset View"; flat: true }
                        Button { text: "↻"; flat: true }
                    }
                }

                // Video/Audio Player (mpv integration)
                Rectangle {
                    visible: peekOverlay.peekType === "Video" || peekOverlay.peekType === "Image"
                    anchors.fill: parent; color: "black"
                    Image { anchors.fill: parent; anchors.margins: 10; source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectFit }
                    Rectangle { visible: peekOverlay.peekType === "Video"; anchors.centerIn: parent; width: 60; height: 60; radius: 30; color: "#aa000000"; Label { anchors.centerIn: parent; text: "▶"; color: "white"; font.pixelSize: 24 } }
                }

                // Text/Code/Hex/Schema/Tree Viewer (ScrollView + TextArea)
                ScrollView {
                    visible: ["DatabaseSchema", "BinaryAssembly", "ArchiveTree", "TextCode"].indexOf(peekOverlay.peekType) >= 0
                    anchors.fill: parent; anchors.margins: 10; clip: true
                    TextArea {
                        text: peekOverlay.contentPayload
                        color: peekOverlay.peekType === "BinaryAssembly" ? "#00ff00" :
                               peekOverlay.peekType === "DatabaseSchema" ? "#ffaa00" : "#d4d4d4"
                        font.family: "Consolas,monospace"; font.pixelSize: 13
                        background: null; readOnly: true; wrapMode: TextArea.Wrap
                    }
                }
            }

            // ── Bottom Metadata Bar ────────────────────────────────────────────
            Rectangle {
                Layout.fillWidth: true; height: 30; color: "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
                    Label { text: peekOverlay.metaDataText; color: "#888"; font.pixelSize: 11; font.family: "Consolas" }
                    Item { Layout.fillWidth: true }
                    Label { text: "Space: Close"; color: "#555"; font.pixelSize: 10 }
                }
            }
        }
    }
}
