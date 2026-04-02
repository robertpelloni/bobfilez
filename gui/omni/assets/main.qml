import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Omni.File 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1200
    height: 800
    title: "File Explorer"
    color: "#1c1c1c" // Windows 11 Dark Mode Background

    FileModel {
        id: fileModel
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // TABS BAR (Top)
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "#1c1c1c"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                spacing: 2

                Rectangle {
                    width: 200
                    height: 32
                    color: "#2c2c2c"
                    radius: 4
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        Label { text: "📁 " + fileModel.currentPath.split('/').pop(); color: "white"; Layout.fillWidth: true; elide: Text.ElideRight }
                        Label { text: "✕"; color: "#999" }
                    }
                }
                
                Button {
                    text: "+"
                    flat: true
                    contentItem: Label { text: "+"; color: "white"; font.pixelSize: 18; horizontalAlignment: Text.AlignHCenter }
                }
            }
        }

        // TOOLBAR & ADDRESS BAR
        Rectangle {
            Layout.fillWidth: true
            height: 90
            color: "#202020"
            border.color: "#333"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Action Icons (Standard Win11 Toolbar)
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45
                    anchors.leftMargin: 15
                    spacing: 20

                    Label { text: "➕ New"; color: "white"; font.bold: true }
                    Rectangle { width: 1; height: 20; color: "#444" }
                    Label { text: "✂️"; color: "#888" }
                    Label { text: "📋"; color: "#888" }
                    Label { text: "🔗"; color: "#888" }
                    Label { text: "🗑️"; color: "#888" }
                    Rectangle { width: 1; height: 20; color: "#444" }
                    Label { text: "⇅ Sort"; color: "white" }
                    Label { text: "⊞ View"; color: "white" }
                    
                    Rectangle { width: 1; height: 20; color: "#444" }
                    
                    Button {
                        id: dedupeBtn
                        text: "✨ Deduplicate"
                        flat: true
                        onClicked: fileModel.findDuplicates()
                        contentItem: Label { 
                            text: "✨ Deduplicate"
                            color: dedupeBtn.down ? "#aaa" : "white"
                            font.bold: true
                        }
                    }

                    Item { Layout.fillWidth: true }
                    Label { text: "⋯"; color: "white"; font.pixelSize: 20; Layout.rightMargin: 15 }
                }

                // Address Bar & Search
                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45
                    anchors.margins: 8
                    spacing: 10

                    Button {
                        text: "←"
                        onClicked: fileModel.goUp()
                        background: Rectangle { color: "transparent" }
                        contentItem: Label { text: "←"; color: "white"; font.pixelSize: 20 }
                    }
                    Button {
                        text: "→"
                        background: Rectangle { color: "transparent" }
                        contentItem: Label { text: "→"; color: "#555"; font.pixelSize: 20 }
                    }
                    Button {
                        text: "↻"
                        onClicked: fileModel.refresh()
                        background: Rectangle { color: "transparent" }
                        contentItem: Label { text: "↻"; color: "white"; font.pixelSize: 18 }
                    }

                    // Breadcrumb Address Bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: "#2c2c2c"
                        radius: 4
                        border.color: "#444"
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            Label { text: "🏠 " + fileModel.currentPath; color: "#ccc"; elide: Text.ElideRight }
                        }
                    }

                    // Search Bar
                    Rectangle {
                        width: 250
                        height: 32
                        color: "#2c2c2c"
                        radius: 4
                        border.color: "#444"
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            Label { text: "🔍 Search"; color: "#888" }
                        }
                    }
                }
            }
        }

        // MAIN CONTENT (Sidebar + File List)
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // SIDEBAR
            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                color: "#202020"
                
                ListView {
                    anchors.fill: parent
                    anchors.margins: 10
                    model: ["Home", "Gallery", "Desktop", "Documents", "Downloads", "Music", "Pictures", "Videos"]
                    delegate: ItemDelegate {
                        width: parent.width
                        height: 36
                        text: modelData
                        contentItem: Label {
                            text: modelData
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 13
                        }
                        background: Rectangle {
                            color: highlighted ? "#333" : "transparent"
                            radius: 4
                        }
                    }
                }
            }

            // Separator
            Rectangle { width: 1; Layout.fillHeight: true; color: "#333" }

            // FILE LIST
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#1c1c1c"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Header
                    RowLayout {
                        Layout.fillWidth: true
                        height: 32
                        anchors.leftMargin: 15
                        spacing: 0
                        Label { text: "Name"; color: "#888"; Layout.preferredWidth: 400; font.pixelSize: 12 }
                        Label { text: "Date modified"; color: "#888"; Layout.preferredWidth: 200; font.pixelSize: 12 }
                        Label { text: "Type"; color: "#888"; Layout.preferredWidth: 100; font.pixelSize: 12 }
                        Label { text: "Size"; color: "#888"; Layout.fillWidth: true; font.pixelSize: 12 }
                    }

                    ListView {
                        id: fileListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: fileModel
                        delegate: ItemDelegate {
                            width: fileListView.width
                            height: 36
                            onDoubleClicked: {
                                if (isDirectory) fileModel.openFolder(fileModel.currentPath + "/" + fileName)
                            }
                            
                            contentItem: RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 15
                                spacing: 0
                                
                                RowLayout {
                                    Layout.preferredWidth: 400
                                    spacing: 10
                                    Label { 
                                        text: isDuplicate ? "👯" : (isDirectory ? "📁" : "📄")
                                        font.pixelSize: 16 
                                    }
                                    Label { 
                                        text: fileName
                                        color: isDuplicate ? "#ff6b6b" : "white" 
                                        font.pixelSize: 13
                                        font.bold: isDuplicate
                                        elide: Text.ElideRight 
                                    }
                                }
                                Label { text: dateModified; color: "#888"; Layout.preferredWidth: 200; font.pixelSize: 13 }
                                Label { text: fileType; color: "#888"; Layout.preferredWidth: 100; font.pixelSize: 13 }
                                Label { text: fileSize; color: "#888"; Layout.fillWidth: true; font.pixelSize: 13 }
                            }
                            
                            background: Rectangle {
                                color: highlighted ? "#2a2a2c" : "transparent"
                                radius: 4
                                border.color: highlighted ? "#0078d4" : "transparent"
                                border.width: 1
                            }
                        }
                    }
                }
            }
        }

        // STATUS BAR
        Rectangle {
            Layout.fillWidth: true
            height: 28
            color: "#1c1c1c"
            border.color: "#333"
            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                Label { text: fileListView.count + " items"; color: "#888"; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
                Label { text: "🌐 English"; color: "#888"; font.pixelSize: 11 }
            }
        }
    }
}
