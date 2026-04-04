import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// TimeMachinePanel.qml — OmniTimeMachine File Reversion UI.
/// Scrubs backwards through the binary deltas of a file, showing file
/// history across modifications without taking up the space of full copies.

Rectangle {
    id: tmPanel
    color: "#0f0f0f"; radius: 8

    property string targetFile: "/Users/robert/Photos/sunset.jpg"
    property int selectedIndex: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "⏳ OmniTimeMachine"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Continuous Data Protection"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: "💾 Snapshot Now"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
            }
        }

        // ── Current Target File ───────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 50; radius: 8; color: "#161616"; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 15
                Label { text: "🎯 Target:"; color: "#888"; font.bold: true; font.pixelSize: 11 }
                TextInput { 
                    text: tmPanel.targetFile; color: "white"; font.family: "Consolas"; font.pixelSize: 13
                    Layout.fillWidth: true; readOnly: true 
                }
                Button { text: "Browse..."; flat: true; contentItem: Label { text: parent.text; color: "#0078d4" } }
            }
        }

        // ── Main Layout: Visualizer & Timeline ────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 20

            // Left: Current Preview / Restore Actions
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; color: "black"; radius: 10
                border.color: "#333"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 15; spacing: 10
                    
                    // Mock Image Preview (Can be anything: text doc, video frame)
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; color: "#111"; radius: 6
                        Label { anchors.centerIn: parent; text: "Preview of version at 2024-04-03 14:00"; color: "#666" }
                        Image { anchors.fill: parent; anchors.margins: 4; source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectFit }
                    }

                    // Restore Controls
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter; spacing: 15
                        Button {
                            text: "Restore In Place"
                            background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                            contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true }
                        }
                        Button {
                            text: "Restore as Copy..."
                            background: Rectangle { color: "#252525"; radius: 6; border.color: "#444" }
                            contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                        }
                        Button { text: "Compare (Diff) with Current"; flat: true; contentItem: Label { text: parent.text; color: "#0078d4" } }
                    }
                }
            }

            // Right: The Version Timeline (Librsync Deltas)
            Rectangle {
                width: 320; Layout.fillHeight: true; color: "#161616"; radius: 10; border.color: "#333"
                
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 10
                    Label { text: "VERSION HISTORY"; color: "#888"; font.bold: true; font.pixelSize: 11; Layout.bottomMargin: 10 }
                    
                    ListView {
                        id: timelineList
                        Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 5
                        model: [
                            {id: "rev-103", date: "Today 16:30", reason: "Auto-Save", size: "2.4 MB", delta: "+7 KB", isCurrent: true},
                            {id: "rev-102", date: "Today 14:00", reason: "Before Photoshop Batch", size: "2.39 MB", delta: "+3 KB", isCurrent: false},
                            {id: "rev-101", date: "Yesterday", reason: "Auto-Save", size: "2.4 MB", delta: "+0.5 KB", isCurrent: false},
                            {id: "rev-100", date: "Apr 1, 2026", reason: "Initial Import", size: "2.4 MB", delta: "2.4 MB (Base)", isCurrent: false}
                        ]

                        delegate: Rectangle {
                            width: parent.width; height: 75; radius: 6
                            color: tmPanel.selectedIndex === index ? "#1e2a3a" : (itemHover.hovered ? "#252525" : "transparent")
                            border.color: tmPanel.selectedIndex === index ? "#0078d4" : "transparent"
                            
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 10; spacing: 10
                                
                                // Timeline Node
                                Column {
                                    Rectangle { width: 2; height: 15; color: index === 0 ? "transparent" : "#444"; anchors.horizontalCenter: parent.horizontalCenter }
                                    Rectangle { width: 10; height: 10; radius: 5; color: modelData.isCurrent ? "#4caf50" : "#0078d4"; anchors.horizontalCenter: parent.horizontalCenter }
                                    Rectangle { width: 2; height: 35; color: index === timelineList.count - 1 ? "transparent" : "#444"; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                                
                                // Data
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    RowLayout {
                                        Label { text: modelData.date; color: modelData.isCurrent ? "#4caf50" : "white"; font.bold: true; font.pixelSize: 13; Layout.fillWidth: true }
                                        Label { text: modelData.isCurrent ? "CURRENT" : ""; color: "#4caf50"; font.pixelSize: 9; font.bold: true }
                                    }
                                    Label { text: modelData.reason; color: "#aaa"; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
                                    RowLayout {
                                        Label { text: modelData.size; color: "#666"; font.pixelSize: 10; font.family: "Consolas" }
                                        Item { Layout.fillWidth: true }
                                        Label { text: modelData.delta; color: "#0078d4"; font.pixelSize: 10; font.family: "Consolas"; ToolTip.visible: itemHover.hovered; ToolTip.text: "Binary Delta Size" }
                                    }
                                }
                            }
                            HoverHandler { id: itemHover }
                            MouseArea { anchors.fill: parent; onClicked: tmPanel.selectedIndex = index }
                        }
                    }
                }
            }
        }
    }
}
