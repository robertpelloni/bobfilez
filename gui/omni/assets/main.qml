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

    // GLOBAL SHELL STATE
    QtObject {
        id: shell
        property bool startMenuOpen: false
        property bool dashboardOpen: false
        property string activePanel: "explorer" // explorer, rename, convert, search, hex, image, md, watcher

        function toggleStartMenu() { startMenuOpen = !startMenuOpen; if(startMenuOpen) dashboardOpen = false }
        function toggleDashboard() { dashboardOpen = !dashboardOpen; if(dashboardOpen) startMenuOpen = false }
        function openPanel(name) { activePanel = name; startMenuOpen = false }
        function openFolder(path) { fileModel.openFolder(path); openPanel("explorer") }
    }

    // DESKTOP ICONS
    DesktopIcons {
        z: 1
    }

    // WINDOW MANAGER
    Item {
        id: windowArea
        anchors.fill: parent
        anchors.bottomMargin: 48
        z: 10

        // Explorer Window
        ExplorerWindow {
            visible: shell.activePanel === "explorer"
            anchors.fill: parent; anchors.margins: 40
        }

        // Search Tool Window
        SearchPanel {
            visible: shell.activePanel === "search"
            anchors.fill: parent; anchors.margins: 40
            onOpenFile: (p) => shell.openFolder(p)
        }

        // Batch Rename Window
        BatchRenamePanel {
            visible: shell.activePanel === "rename"
            anchors.fill: parent; anchors.margins: 40
        }

        // Batch Convert Window
        BatchConvertPanel {
            visible: shell.activePanel === "convert"
            anchors.fill: parent; anchors.margins: 40
        }

        // Hex Editor Window
        HexEditorPanel {
            visible: shell.activePanel === "hex"
            anchors.fill: parent; anchors.margins: 40
        }

        // Image Viewer Window
        ImageViewerPanel {
            visible: shell.activePanel === "image"
            anchors.fill: parent; anchors.margins: 40
        }

        // Markdown Viewer Window
        MarkdownViewerPanel {
            visible: shell.activePanel === "md"
            anchors.fill: parent; anchors.margins: 40
        }
        
        // File Watcher Window
        FileWatcherPanel {
            visible: shell.activePanel === "watcher"
            anchors.fill: parent; anchors.margins: 40
        }

        // Enhanced File Ops (The Multi-Tool)
        EnhancedFileOpsPanel {
            visible: shell.activePanel === "fileops"
            anchors.fill: parent; anchors.margins: 40
        }

        // Visual Dedup (The Swipe Tool)
        VisualDedupPanel {
            visible: shell.activePanel === "visual_dedup"
            anchors.fill: parent; anchors.margins: 40
        }

        // Data Topology Map (Tree-map)
        TopologyPanel {
            visible: shell.activePanel === "topology"
            anchors.fill: parent; anchors.margins: 40
        }
    }

    // TASKBAR
    Taskbar {
        id: taskbar
        z: 100
    }

    // START MENU
    StartMenu {
        id: startMenu
        visible: shell.startMenuOpen
        anchors.bottom: taskbar.top
        anchors.bottomMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        z: 101
    }

    // DASHBOARD
    Dashboard {
        id: dashboard
        visible: shell.dashboardOpen
        anchors.bottom: taskbar.top
        anchors.right: parent.right
        anchors.margins: 12
        z: 102
    }
}
