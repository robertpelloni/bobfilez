import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// CloudPanel.qml — Native Cloud Integration and Sync Management.
/// Manage S3 buckets, Google Drive accounts, and Azure storage.
/// Part of the "Universal Data Custodian" suite.

Rectangle {
    id: cloudPanel
    color: "#0f0f0f"; radius: 8

    property var accounts: [] // [{id, name, provider, connected}]
    property bool isSyncing: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "☁️ Cloud Storage"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Native Explorer Integration"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: "＋ Add Account"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: addAccountMenu.open()
                Menu {
                    id: addAccountMenu
                    MenuItem { text: "AWS S3 Bucket"; icon.source: "qrc:/icons/s3.png" }
                    MenuItem { text: "Google Drive"; icon.source: "qrc:/icons/gdrive.png" }
                    MenuItem { text: "Azure Blob Storage"; icon.source: "qrc:/icons/azure.png" }
                    MenuItem { text: "Backblaze B2" }
                    MenuItem { text: "Dropbox" }
                }
            }
        }

        // ── Connected Accounts Grid ───────────────────────────────────────
        GroupBox {
            label: Label { text: "Active Cloud Drives"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 15

            GridView {
                anchors.fill: parent; cellWidth: 300; cellHeight: 120; clip: true
                model: [
                    {id: "s3-01", name: "Backup-Archive-US", provider: "AWS S3", region: "us-east-1", status: "Connected", size: "2.4 TB"},
                    {id: "gd-01", name: "Personal Drive", provider: "Google Drive", region: "Global", status: "Connected", size: "15 GB"},
                    {id: "az-01", name: "Client-Assets", provider: "Azure", region: "West Europe", status: "Disconnected", size: "Unknown"}
                ]
                
                delegate: Rectangle {
                    width: 280; height: 100; radius: 10
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 15; spacing: 15
                        
                        Rectangle { width: 50; height: 50; radius: 25; color: "#0d0d0d"
                            Label { anchors.centerIn: parent; text: modelData.provider[0]; font.pixelSize: 24; color: "#0078d4" } }
                        
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.name; color: "white"; font.bold: true; font.pixelSize: 13 }
                            Label { text: modelData.provider + " • " + modelData.region; color: "#888"; font.pixelSize: 10 }
                            Label { text: modelData.size; color: "#4caf50"; font.pixelSize: 11; font.bold: true }
                        }
                        
                        Column {
                            spacing: 4
                            Rectangle { width: 8; height: 8; radius: 4; color: modelData.status === "Connected" ? "#4caf50" : "#ff4444"; anchors.horizontalCenter: parent.horizontalCenter }
                            Button { text: "⚙️"; flat: true; contentItem: Label { text: parent.text; color: "#666" } }
                        }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── Sync Rules ────────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Cloud Sync & Mirroring Rules"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; height: 200
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 5
                model: [
                    {local: "/Users/robert/Documents", remote: "s3://Backup-Archive-US/Docs", type: "Two-way", interval: "Hourly"},
                    {local: "/Users/robert/Pictures/Summer", remote: "gd://Personal/Summer_2024", type: "Mirror", interval: "Real-time"}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 40; radius: 4; color: "#1a1a1a"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 15
                        Label { text: "📁 " + modelData.local; color: "#ccc"; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideMiddle }
                        Label { text: "➔"; color: "#0078d4" }
                        Label { text: "☁️ " + modelData.remote; color: "#ccc"; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideMiddle }
                        Rectangle { width: 60; height: 20; radius: 10; color: "#0078d422"; border.color: "#0078d4"
                            Label { anchors.centerIn: parent; text: modelData.type; color: "#0078d4"; font.pixelSize: 9 } }
                        Label { text: modelData.interval; color: "#666"; font.pixelSize: 10; width: 60 }
                        Button { text: "▶"; flat: true; contentItem: Label { text: parent.text; color: "#4caf50" } }
                    }
                }
            }
        }

        // ── Footer ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "Network Usage:"; color: "#888"; font.pixelSize: 11 }
            ProgressBar { value: 0.15; Layout.fillWidth: true; background: Rectangle { color: "#1a1a1a"; radius: 4; implicitHeight: 6 } }
            Label { text: "1.2 MB/s ↑  240 KB/s ↓"; color: "#0078d4"; font.pixelSize: 11; font.bold: true }
        }
    }
}
