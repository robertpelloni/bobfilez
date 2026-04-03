import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects 1.15
import Omni.File 1.0

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1400; height: 900
    title: "Omni Shell"
    
    // DESKTOP WALLPAPER simulation
    background: Rectangle {
        color: "#000"
        LinearGradient {
            anchors.fill: parent
            start: Qt.point(0, 0); end: Qt.point(width, height)
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1e3a8a" } // Windows 11 Blue
                GradientStop { position: 1.0; color: "#000000" }
            }
        }
    }

    FileModel { id: fileModel }

    // DESKTOP ICONS
    DesktopIcons {
        z: 1
    }

    // EXPLORER WINDOW
    Rectangle {
        id: explorerWindow
        width: 1150; height: 800
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2 - 40
        radius: 10
        color: "#1c1c1c"
        border.color: "#333"
        z: 10
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
                        Rectangle {
                            width: 34; height: 34; color: "transparent"; radius: 6
                            anchors.verticalCenter: parent.verticalCenter
                            Label { anchors.centerIn: parent; text: "＋"; color: "white" }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; onEntered: parent.color = "#3d3d3d"; onExited: parent.color = "transparent" }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Window Buttons
                    Row {
                        spacing: 0
                        Rectangle { width: 46; height: 40; color: "transparent"
                            Label { anchors.centerIn: parent; text: "－"; color: "white" }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; onEntered: parent.color = "#3d3d3d"; onExited: parent.color = "transparent" }
                        }
                        Rectangle { width: 46; height: 40; color: "transparent"
                            Label { anchors.centerIn: parent; text: "▢"; color: "white" }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; onEntered: parent.color = "#3d3d3d"; onExited: parent.color = "transparent" }
                        }
                        Rectangle { width: 46; height: 40; color: "transparent"
                            Label { anchors.centerIn: parent; text: "✕"; color: "white" }
                            MouseArea { anchors.fill: parent; hoverEnabled: true; onEntered: parent.color = "#e81123"; onExited: parent.color = "transparent" }
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent; anchors.rightMargin: 140
                    property point lastMousePos
                    onPressed: lastMousePos = Qt.point(mouse.x, mouse.y)
                    onPositionChanged: {
                        let delta = Qt.point(mouse.x - lastMousePos.x, mouse.y - lastMousePos.y)
                        explorerWindow.x += delta.x
                        explorerWindow.y += delta.y
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
                        Button { flat: true; text: "→"; font.pixelSize: 18; enabled: false }
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
                                color: "white"
                                Layout.fillWidth: true
                                clip: true
                                selectByMouse: true
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
                                id: searchInput; Layout.fillWidth: true; color: "white"; font.pixelSize: 13
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
                    Layout.preferredWidth: 220; Layout.fillHeight: true; color: "#1c1c1c"
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 5
                        Repeater {
                            model: [
                                { icon: "⭐", text: "Quick access" },
                                { icon: "☁️", text: "OneDrive" },
                                { icon: "💻", text: "This PC" },
                                { icon: "🌐", text: "Network" }
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
                                    else if (modelData.text === "Network") fileModel.openFolder("//localhost")
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
                            Label { text: "Name"; color: "#888"; font.pixelSize: 12; Layout.fillWidth: true; MouseArea { anchors.fill: parent; onClicked: fileModel.sortBy("Name", true) } }
                            Label { text: "Date modified"; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 150; MouseArea { anchors.fill: parent; onClicked: fileModel.sortBy("Date modified", false) } }
                            Label { text: "Type"; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 100; MouseArea { anchors.fill: parent; onClicked: fileModel.sortBy("Type", true) } }
                            Label { text: "Size"; color: "#888"; font.pixelSize: 12; Layout.preferredWidth: 100; MouseArea { anchors.fill: parent; onClicked: fileModel.sortBy("Size", false) } }
                        }
                    }

                    ListView {
                        id: fileListView; Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: fileModel
                        delegate: ItemDelegate {
                            width: fileListView.width; height: 32
                            background: Rectangle { 
                                color: isSelected ? "#3d3d3d" : (hovered ? "#2d2d2d" : "transparent")
                                radius: 4
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: function(mouse) {
                                    if (mouse.button === Qt.LeftButton) {
                                        fileModel.toggleSelection(index)
                                    } else if (mouse.button === Qt.RightButton) {
                                        fileModel.toggleSelection(index)
                                        fileContextMenu.popup()
                                    }
                                }
                                onDoubleClicked: function(mouse) {
                                    if (mouse.button === Qt.LeftButton && isDirectory) {
                                        fileModel.openFolder(fileModel.currentPath + "/" + fileName)
                                    }
                                }
                            }

                            Menu {
                                id: fileContextMenu
                                MenuItem { text: "Open" }
                                MenuItem { text: "Copy" }
                                MenuItem { text: "Delete" }
                                MenuSeparator {}
                                MenuItem { text: "Properties" }
                            }

                            contentItem: RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 15; spacing: 0
                                RowLayout {
                                    Layout.fillWidth: true; spacing: 10
                                    Label { text: isDuplicate ? "👯" : (isDirectory ? "📁" : "📄"); font.pixelSize: 16 }
                                    Label { text: fileName; color: isDuplicate ? "#ff6b6b" : "white"; font.pixelSize: 13; elide: Text.ElideRight }
                                }
                                Label { text: dateModified; color: "#888"; font.pixelSize: 13; Layout.preferredWidth: 150 }
                                Label { text: fileType; color: "#888"; font.pixelSize: 13; Layout.preferredWidth: 100 }
                                Label { text: fileSize; color: "#888"; font.pixelSize: 13; Layout.preferredWidth: 100 }
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
                    Rectangle { width: 1; height: 12; color: "#444" }
                    Label { text: fileModel.selectedCount + " items selected"; color: "#888"; font.pixelSize: 11 }
                    Item { Layout.fillWidth: true }
                    Label { text: "✨ OmniEngine Active"; color: "#0078d4"; font.pixelSize: 11 }
                }
            }
        }
    }

    // TASKBAR
    Rectangle {
        id: taskbar
        anchors.bottom: parent.bottom
        width: parent.width; height: 48
        color: "#1c1c1c"; opacity: 0.95; border.color: "#333"
        z: 100
        RowLayout {
            anchors.centerIn: parent; spacing: 10
            Button { flat: true; onClicked: startMenu.visible = !startMenu.visible; contentItem: Label { text: "⊞"; color: "#0078d4"; font.pixelSize: 24 } }
            Repeater {
                model: ["🔍", "📁", "✨", "⚙️"]
                Button { flat: true; contentItem: Label { text: modelData; font.pixelSize: 20 } }
            }
        }
        RowLayout {
            anchors.right: parent.right; anchors.rightMargin: 15; anchors.verticalCenter: parent.verticalCenter
            spacing: 15
            Label { text: "🌐"; color: "white" }
            MouseArea {
                width: 80; height: 40
                Column { Label { text: "11:11 PM"; color: "white"; font.pixelSize: 11 }; Label { text: "4/1/2026"; color: "white"; font.pixelSize: 11 } }
                onClicked: dashboard.visible = !dashboard.visible
            }
        }
    }

    // START MENU
    StartMenu {
        id: startMenu
        visible: false
        anchors.bottom: taskbar.top
        anchors.bottomMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        z: 101
    }

    // DASHBOARD
    Dashboard {
        id: dashboard
        visible: false
        anchors.bottom: taskbar.top
        anchors.right: parent.right
        anchors.margins: 12
        z: 102
    }
}
