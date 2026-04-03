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

        // Digital Rot Agent
        DigitalRotPanel {
            visible: shell.activePanel === "pruner"
            anchors.fill: parent; anchors.margins: 40
        }

        // Auto-Hierarchy Generator
        HierarchyPanel {
            visible: shell.activePanel === "hierarchy"
            anchors.fill: parent; anchors.margins: 40
        }

        // Gamification Dashboard (XP/Achievements)
        GamificationPanel {
            visible: shell.activePanel === "achievements"
            anchors.fill: parent; anchors.margins: 40
        }

        // Cloud Storage Manager
        CloudPanel {
            visible: shell.activePanel === "cloud"
            anchors.fill: parent; anchors.margins: 40
        }

        // Network Manager (SSH/SMB)
        NetworkPanel {
            visible: shell.activePanel === "network"
            anchors.fill: parent; anchors.margins: 40
        }

        // Secure Vault
        VaultPanel {
            visible: shell.activePanel === "vault"
            anchors.fill: parent; anchors.margins: 40
        }

        // Forensic Audit Ledger
        ForensicPanel {
            visible: shell.activePanel === "forensic"
            anchors.fill: parent; anchors.margins: 40
        }

        // Asset Manager (MAM)
        AssetManagerPanel {
            visible: shell.activePanel === "assets"
            anchors.fill: parent; anchors.margins: 20
        }

        // OmniVision NLE (Video Editing)
        OmniVisionPanel {
            visible: shell.activePanel === "omnivision"
            anchors.fill: parent; anchors.margins: 20
        }

        // OmniAudio DAW (Music Production)
        OmniAudioPanel {
            visible: shell.activePanel === "omniaudio"
            anchors.fill: parent; anchors.margins: 20
        }

        // OmniGraph (Semantic Knowledge Graph)
        OmniGraphPanel {
            visible: shell.activePanel === "omnigraph"
            anchors.fill: parent; anchors.margins: 20
        }

        // OmniFlow (Visual Automations)
        OmniFlowPanel {
            visible: shell.activePanel === "omniflow"
            anchors.fill: parent; anchors.margins: 40
        }

        // OmniTimeMachine (CDP / Versioning)
        TimeMachinePanel {
            visible: shell.activePanel === "timemachine"
            anchors.fill: parent; anchors.margins: 40
        }

        // Data Swarm (Distributed Sync)
        SwarmPanel {
            visible: shell.activePanel === "swarm"
            anchors.fill: parent; anchors.margins: 40
        }

        // Data Recovery & Healing
        RecoveryPanel {
            visible: shell.activePanel === "recovery"
            anchors.fill: parent; anchors.margins: 40
        }

        // Digital Notary
        NotaryPanel {
            visible: shell.activePanel === "notary"
            anchors.fill: parent; anchors.margins: 40
        }

        // Photo Library (Apple/Google Photos)
        PhotoLibraryPanel {
            visible: shell.activePanel === "photos"
            anchors.fill: parent; anchors.margins: 20
        }

        // Photo Develop (Lightroom)
        PhotoDevelopPanel {
            visible: shell.activePanel === "develop"
            anchors.fill: parent; anchors.margins: 20
        }

        // OmniOracle (RAG Copilot)
        OraclePanel {
            visible: shell.activePanel === "oracle"
            anchors.fill: parent; anchors.margins: 40
        }

        // OmniMount (Dokany/FUSE Native Drive Mounter)
        OmniMountPanel {
            visible: shell.activePanel === "omnimount"
            anchors.fill: parent; anchors.margins: 40
        }

        // OmniSec (Cybersecurity Suite)
        OmniSecPanel {
            visible: shell.activePanel === "omnisec"
            anchors.fill: parent; anchors.margins: 40
        }

        // Notification Center Overlay
        Rectangle {
            id: notificationCenter
            visible: shell.dashboardOpen
            anchors.right: parent.right; anchors.top: parent.top
            anchors.margins: 20; width: 350; height: 600
            color: "#e61a1a1a"; radius: 12; border.color: "#33ffffff"
            z: 1000

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 20; spacing: 15
                Label { text: "Notifications"; color: "white"; font.pixelSize: 18; font.bold: true }
                
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10; clip: true
                    model: [
                        {icon: "🔥", title: "Visual Discovery", body: "Found 12 near-duplicate photos in /Summer."},
                        {icon: "🛡️", title: "Security Alert", body: "Detected PII in 'invoice.pdf'. Move to Vault?"},
                        {icon: "🧹", title: "Shadow Sorter", body: "Identified 4.2 GB of obsolete installers."},
                        {icon: "✅", title: "Sync Complete", body: "Mirroring to OneDrive finished successfully."}
                    ]
                    delegate: Rectangle {
                        width: parent.width; height: 80; radius: 8; color: "#22ffffff"
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 12
                            Label { text: modelData.icon; font.pixelSize: 24 }
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Label { text: modelData.title; color: "white"; font.bold: true; font.pixelSize: 13 }
                                Label { text: modelData.body; color: "#aaa"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            }
                        }
                    }
                }
                
                Button {
                    text: "Clear All"
                    Layout.fillWidth: true; flat: true
                    contentItem: Label { text: parent.text; color: "#888"; horizontalAlignment: Text.AlignHCenter }
                }
            }
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
