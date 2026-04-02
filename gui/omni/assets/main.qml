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
        width: 1100; height: 750
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2 - 50
        radius: 8
        color: "#1c1c1c"
        border.color: "#333"
        z: 10
        visible: true
        
        // Window Controls and Title
        MouseArea {
            height: 48; width: parent.width
            property point lastMousePos
            onPressed: lastMousePos = Qt.point(mouse.x, mouse.y)
            onPositionChanged: {
                let delta = Qt.point(mouse.x - lastMousePos.x, mouse.y - lastMousePos.y)
                explorerWindow.x += delta.x
                explorerWindow.y += delta.y
            }
        }

        ColumnLayout {
            anchors.fill: parent; spacing: 0
            
            // ... (The entire explorer UI code from before goes here) ...
            // Simplified for this unified file:
            Rectangle { Layout.fillWidth: true; height: 48; color: "transparent"
                RowLayout { anchors.fill: parent; anchors.leftMargin: 15; Label { text: "📁 File Explorer"; color: "white"; font.bold: true } }
            }
            
            Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }
            
            // Toolbar (Condensed)
            Rectangle { Layout.fillWidth: true; height: 48; color: "#202020"
                RowLayout { anchors.fill: parent; anchors.leftMargin: 15; spacing: 20
                    Label { text: "➕ New"; color: "white"; font.bold: true }
                    Rectangle { width: 1; height: 20; color: "#444" }
                    Label { text: "✂️"; color: "#888" }; Label { text: "📋"; color: "#888" }
                    Item { Layout.fillWidth: true }
                    Button { text: "✨ Deduplicate"; onClicked: fileModel.findDuplicates() }
                }
            }

            // Main List Area
            RowLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
                Rectangle { Layout.preferredWidth: 200; Layout.fillHeight: true; color: "#202020"; opacity: 0.9 }
                Rectangle { width: 1; Layout.fillHeight: true; color: "#333" }
                ListView {
                    id: fileListView; Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: fileModel
                    delegate: ItemDelegate {
                        width: fileListView.width; height: 38
                        contentItem: RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 15
                            Label { text: isDuplicate ? "👯" : (isDirectory ? "📁" : "📄"); font.pixelSize: 18 }
                            Label { text: fileName; color: isDuplicate ? "#ff6b6b" : "white"; Layout.fillWidth: true }
                            Label { text: fileSize; color: "#888"; Layout.preferredWidth: 100 }
                        }
                    }
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
