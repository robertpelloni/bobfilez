import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// VaultPanel.qml — Secure interface for the Encrypted Vault.
/// Manage files protected by AES-256-GCM.
/// Part of the "Privacy Sentinel" suite.

Rectangle {
    id: vaultPanel
    color: "#0a0a0a"; radius: 8

    property bool isLocked: true
    property string vaultPath: "C:/Users/robert/.vault"
    property var vaultFiles: [] // [{id, name, size, date}]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🔒 Secure Vault"; font.pixelSize: 22; font.bold: true; color: "white" }
            Rectangle { width: 60; height: 24; radius: 12; color: vaultPanel.isLocked ? "#3a1a1a" : "#1a3a1a"
                Label { anchors.centerIn: parent; text: vaultPanel.isLocked ? "LOCKED" : "OPEN"; color: vaultPanel.isLocked ? "#ff4444" : "#4caf50"; font.pixelSize: 10; font.bold: true } }
            Item { Layout.fillWidth: true }
            Button {
                text: vaultPanel.isLocked ? "Unlock Vault" : "Lock Vault"
                background: Rectangle { color: vaultPanel.isLocked ? "#0078d4" : "#dc2626"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: vaultPanel.isLocked = !vaultPanel.isLocked
            }
        }

        // ── Auth Area (When Locked) ───────────────────────────────────────
        ColumnLayout {
            visible: vaultPanel.isLocked
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 50
            spacing: 20
            
            Label { text: "🔐"; font.pixelSize: 64; Layout.alignment: Qt.AlignHCenter }
            Label { text: "Vault is Encrypted"; color: "#888"; font.pixelSize: 16; Layout.alignment: Qt.AlignHCenter }
            
            TextField {
                id: passwordField
                placeholderText: "Enter Master Password"; echoMode: TextInput.Password
                color: "white"; font.pixelSize: 14; Layout.preferredWidth: 300
                background: Rectangle { color: "#1a1a1a"; radius: 6; border.color: "#333" }
            }
            
            Button {
                text: "Authorize Access"
                Layout.preferredWidth: 300; height: 40
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                onClicked: vaultPanel.isLocked = false
            }
        }

        // ── File List (When Unlocked) ─────────────────────────────────────
        GroupBox {
            visible: !vaultPanel.isLocked
            label: Label { text: "Protected Files"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 5
                model: [
                    {name: "tax_return_2023.pdf", size: "2.4 MB", date: "2024-03-15"},
                    {name: "passwords_backup.kdbx", size: "450 KB", date: "2024-04-01"},
                    {name: "ssh_keys_archive.zip", size: "12 KB", date: "2024-01-10"},
                    {name: "identification_scan.jpg", size: "4.2 MB", date: "2023-12-20"}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 50; radius: 6
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        Label { text: "📄"; font.pixelSize: 18 }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.name; color: "white"; font.pixelSize: 13 }
                            Label { text: "Added on " + modelData.date; color: "#666"; font.pixelSize: 10 }
                        }
                        Label { text: modelData.size; color: "#aaa"; font.pixelSize: 12 }
                        Button { text: "🔓"; flat: true; ToolTip.text: "Decrypt & Move Out" }
                        Button { text: "🗑"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444" } }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── Stats (When Unlocked) ─────────────────────────────────────────
        RowLayout {
            visible: !vaultPanel.isLocked
            spacing: 20
            Label { text: "Total Items: 4"; color: "#888"; font.pixelSize: 11 }
            Label { text: "Total Size: 7.1 MB"; color: "#888"; font.pixelSize: 11 }
            Item { Layout.fillWidth: true }
            Label { text: "Encryption: AES-256-GCM"; color: "#4caf50"; font.pixelSize: 11; font.bold: true }
        }
    }
}
