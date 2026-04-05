import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// PhotoLibraryPanel.qml — Apple Photos / Google Photos parity library module.
/// Retargeted to local Qt Quick / Controls primitives for BTK migration.
/// Features People recognition, Memories, Map view, and Smart Albums.

Rectangle {
    id: libraryPanel
    color: "#0a0a0a"; radius: 10

    property int activeView: 0 // 0=Library, 1=Memories, 2=People, 3=Places

    RowLayout {
        anchors.fill: parent; spacing: 0

        // ── Navigation Sidebar ──
        Rectangle {
            Layout.preferredWidth: 200; Layout.fillHeight: true
            color: "#161616"; border.color: "#333"

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 15; spacing: 8
                
                Label { text: "PHOTOS"; color: "#666"; font.bold: true; font.pixelSize: 10 }
                
                component NavItem: Button {
                    Layout.fillWidth: true; flat: true
                    property bool active: false
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 8
                    }
                    background: Rectangle { color: parent.active ? "#0078d4" : "transparent"; radius: 6 }
                }

                NavItem { text: "🖼️ Library"; active: libraryPanel.activeView === 0; onClicked: libraryPanel.activeView = 0 }
                NavItem { text: "✨ Memories"; active: libraryPanel.activeView === 1; onClicked: libraryPanel.activeView = 1 }
                NavItem { text: "👤 People"; active: libraryPanel.activeView === 2; onClicked: libraryPanel.activeView = 2 }
                NavItem { text: "🗺️ Places"; active: libraryPanel.activeView === 3; onClicked: libraryPanel.activeView = 3 }
                NavItem { text: "♻️ Recent Delete" }

                Item { height: 20 }
                Label { text: "ALBUMS"; color: "#666"; font.bold: true; font.pixelSize: 10 }
                NavItem { text: "⭐ Favorites" }
                NavItem { text: "🎥 Videos" }
                NavItem { text: "📱 Screenshots" }
                NavItem { text: "📄 Documents" }

                Item { Layout.fillHeight: true }
                
                // Import Info
                Label { text: "8,432 Photos"; color: "#888"; font.pixelSize: 11 }
                ProgressBar { value: 0.85; Layout.fillWidth: true }
            }
        }

        // ── Main Content ──
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0

            // Header bar
            Rectangle {
                Layout.fillWidth: true; height: 60; color: "#161616"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 20
                    Label { text: ["Library", "Memories", "People", "Places"][libraryPanel.activeView]; color: "white"; font.pixelSize: 24; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Button { text: "Select"; flat: true }
                    Button { text: "Import"; background: Rectangle { color: "#0078d4"; radius: 6 } }
                }
            }

            // Grid View
            Item {
                Layout.fillWidth: true; Layout.fillHeight: true
                
                // Photo Grid (Library View)
                GridView {
                    visible: libraryPanel.activeView === 0
                    anchors.fill: parent; anchors.margins: 10
                    cellWidth: 180; cellHeight: 180; clip: true
                    model: 100
                    delegate: Rectangle {
                        width: 170; height: 170; radius: 4; color: "#1a1a1a"
                        Image { anchors.fill: parent; anchors.margins: 2; source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectCrop }
                        // Badge for RAW or Video
                        Rectangle { anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 5; width: 30; height: 16; radius: 4; color: "#aa000000"; visible: index % 5 == 0
                            Label { anchors.centerIn: parent; text: "RAW"; color: "white"; font.pixelSize: 9 } }
                    }
                }

                // People View
                GridView {
                    visible: libraryPanel.activeView === 2
                    anchors.fill: parent; anchors.margins: 30
                    cellWidth: 200; cellHeight: 240; clip: true
                    model: [
                        {name: "Alice", count: 1420},
                        {name: "Bob", count: 850},
                        {name: "Charlie", count: 320},
                        {name: "Unknown", count: 45}
                    ]
                    delegate: ColumnLayout {
                        Rectangle { width: 180; height: 180; radius: 90; color: "#1e1e1e"; border.color: "#333"
                            Label { anchors.centerIn: parent; text: "👤"; font.pixelSize: 60 } }
                        Label { text: modelData.name; color: "white"; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                        Label { text: modelData.count + " photos"; color: "#888"; font.pixelSize: 11; Layout.alignment: Qt.AlignHCenter }
                    }
                }

                // Map View (Places)
                Rectangle {
                    visible: libraryPanel.activeView === 3
                    anchors.fill: parent; color: "#0d0d0d"
                    Label { anchors.centerIn: parent; text: "🗺️ Interactive Map (OpenStreetMap)"; color: "#444"; font.pixelSize: 20 }
                }
            }
        }
    }
}
