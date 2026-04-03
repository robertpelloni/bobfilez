import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Omni.File 1.0

/// ExplorerWindow.qml — High-fidelity Windows 11 style file explorer.
/// Extracted from main.qml for modular shell window management.

Rectangle {
    id: explorerWindow
    radius: 10
    color: "#1c1c1c"
    border.color: "#333"
    clip: true

    // Window Glow/Shadow effect
    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 0
        verticalOffset: 8
        radius: 20
        samples: 25
        color: "#60000000"
    }

    ColumnLayout {
        anchors.fill: parent; spacing: 0
        
        // TITLE BAR & TABS
        Rectangle {
            Layout.fillWidth: true; height: 40; color: "#2d2d2d"
            
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 10; spacing: 0
                
                // Tabs
                Row {
                    spacing: 2
                    Rectangle {
                        width: 180; height: 34; color: "#1c1c1c"; radius: 6
                        anchors.verticalCenter: parent.verticalCenter
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
                            Label { text: "📁 " + fileModel.currentPath.split('/').pop(); color: "white"; elide: Text.ElideRight; Layout.fillWidth: true }
                            Label { text: "✕"; color: "#888"; font.pixelSize: 10 }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Window Buttons
                Row {
                    spacing: 0
                    Rectangle { width: 46; height: 40; color: "transparent"
                        Label { anchors.centerIn: parent; text: "－"; color: "white" }
                    }
                    Rectangle { width: 46; height: 40; color: "transparent"
                        Label { anchors.centerIn: parent; text: "▢"; color: "white" }
                    }
                    Rectangle { width: 46; height: 40; color: "transparent"
                        Label { anchors.centerIn: parent; text: "✕"; color: "white" }
                        MouseArea { anchors.fill: parent; onClicked: shell.openPanel("none") }
                    }
                }
            }
        }

        // TOOLBAR
        Rectangle {
            Layout.fillWidth: true; height: 48; color: "#1c1c1c"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 15; anchors.rightMargin: 15; spacing: 10
                
                Row {
                    spacing: 5
                    Button { flat: true; text: "←"; font.pixelSize: 18; enabled: false }
                    Button { flat: true; text: "↑"; font.pixelSize: 18; onClicked: fileModel.goUp() }
                    Button { flat: true; text: "↻"; font.pixelSize: 18; onClicked: fileModel.refresh() }
                }

                Rectangle { width: 1; height: 24; color: "#444" }

                // Address Bar
                Rectangle {
                    Layout.fillWidth: true; height: 32; color: "#2d2d2d"; radius: 4; border.color: "#444"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10
                        Label { text: "📁"; color: "#888" }
                        TextInput { 
                            text: fileModel.currentPath
                            color: "white"; Layout.fillWidth: true; clip: true; selectByMouse: true
                            onAccepted: fileModel.openFolder(text)
                        }
                    }
                }

                // Search Bar
                Rectangle {
                    Layout.preferredWidth: 250; height: 32; color: "#2d2d2d"; radius: 4; border.color: "#444"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10
                        Label { text: "🔍"; color: "#888" }
                        TextInput { 
                            id: explorerSearch; Layout.fillWidth: true; color: "white"; font.pixelSize: 13
                            placeholderText: "Search " + fileModel.currentPath.split('/').pop()
                            onTextChanged: fileModel.setSearchFilter(text)
                        }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }

        // COMMAND BAR
        Rectangle {
            Layout.fillWidth: true; height: 48; color: "#1c1c1c"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 15; spacing: 20
                Label { text: "➕ New"; color: "white"; font.bold: true }
                Rectangle { width: 1; height: 20; color: "#444" }
                Label { text: "✂️"; color: "#888" }; Label { text: "📋"; color: "#888" }; Label { text: "↗️"; color: "#888" }; Label { text: "🗑️"; color: "#888" }
                Rectangle { width: 1; height: 20; color: "#444" }
                Label { text: "🔀 Sort"; color: "white" }
                Label { text: "≣ View"; color: "white" }
                Item { Layout.fillWidth: true }
                Button { 
                    text: "✨ Deduplicate"; highlighted: true
                    onClicked: fileModel.findDuplicates()
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }

        // MAIN CONTENT AREA
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            
            // Sidebar
            Rectangle {
                Layout.preferredWidth: 200; Layout.fillHeight: true; color: "#1c1c1c"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 5
                    Repeater {
                        model: [
                            { icon: "⭐", text: "Quick access" },
                            { icon: "☁️", text: "Cloud Drives" },
                            { icon: "💻", text: "This PC" },
                            { icon: "🌐", text: "Network (SFTP/SMB)" },
                            { icon: "🔗", text: "Live Folders" },
                            { icon: "🐙", text: "Repositories" }
                        ]
                        ItemDelegate {
                            Layout.fillWidth: true; height: 32
                            contentItem: RowLayout {
                                spacing: 10
                                Label { text: modelData.icon; font.pixelSize: 16 }
                                Label { text: modelData.text; color: "white"; font.pixelSize: 13 }
                            }
                            background: Rectangle { color: hovered ? "#3d3d3d" : "transparent"; radius: 4 }
                            onClicked: {
                                if (modelData.text === "This PC") fileModel.openFolder("C:/")
                                else if (modelData.text === "Cloud Drives") shell.openPanel("cloud")
                                else if (modelData.text === "Network (SFTP/SMB)") shell.openPanel("network")
                                else if (modelData.text === "Repositories") shell.openPanel("omnigit")
                                else if (modelData.text === "Live Folders") shell.openPanel("hierarchy")
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            Rectangle { width: 1; Layout.fillHeight: true; color: "#333" }

            // File List
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
                
                // Header
                Rectangle {
                    Layout.fillWidth: true; height: 32; color: "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 15; spacing: 0
                        Label { text: "Name"; color: "#888"; font.pixelSize: 12; Layout.fillWidth: true }
                        Label { text: "Date modified"; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 150 }
                        Label { text: "Size"; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 100 }
                    }
                }

                ListView {
                    id: fileListView; Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: fileModel
                    delegate: ItemDelegate {
                        width: fileListView.width; height: 32
                        background: Rectangle { color: isSelected ? "#3d3d3d" : (hovered ? "#2d2d2d" : "transparent"); radius: 4 }
                        
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onClicked: function(mouse) {
                                if (mouse.button === Qt.LeftButton) {
                                    fileModel.toggleSelection(index)
                                }
                            }
                            onDoubleClicked: function(mouse) {
                                if (mouse.button === Qt.LeftButton && isDirectory) {
                                    fileModel.openFolder(fileModel.currentPath + "/" + fileName)
                                }
                            }
                        }

                        Keys.onSpacePressed: shell.togglePeek(fileModel.currentPath + "/" + fileName)

                        contentItem: RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 15; spacing: 10
                            Label { text: isDirectory ? "📁" : "📄"; font.pixelSize: 16 }
                            Label { text: fileName; color: "white"; font.pixelSize: 13; Layout.fillWidth: true; elide: Text.ElideRight }
                            Label { text: dateModified; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 150 }
                            Label { text: fileSize; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 100 }
                        }
                    }
                    ScrollBar.vertical: ScrollBar { }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }

        // STATUS BAR
        Rectangle {
            Layout.fillWidth: true; height: 28; color: "#1c1c1c"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 15; anchors.rightMargin: 15
                Label { text: fileModel.rowCount() + " items"; color: "#888"; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
                Label { text: "✨ OmniEngine Active"; color: "#0078d4"; font.pixelSize: 11 }
            }
        }
    }
}
