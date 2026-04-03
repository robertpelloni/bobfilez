import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// NetworkPanel.qml — Remote Storage & Network Drive Manager.
/// Connect to SFTP (SSH), FTP, and SMB (Samba) servers.
/// Part of the "Universal Data Custodian" suite.

Rectangle {
    id: networkPanel
    color: "#0f0f0f"; radius: 8

    property var connections: [] // [{id, host, protocol, active}]
    property bool isConnecting: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🌐 Network Locations"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "SFTP • FTP • SMB • WebDAV"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: "＋ New Connection"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: addConnPopup.open()
            }
        }

        // ── Active Connections List ───────────────────────────────────────
        GroupBox {
            label: Label { text: "Saved Locations"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 8
                model: [
                    {id: "srv-01", host: "192.168.1.50", protocol: "SMB", name: "Home-NAS", user: "robert", status: "Active"},
                    {id: "srv-02", host: "shell.project.org", protocol: "SFTP", name: "Dev-Server", user: "ubuntu", status: "Inactive"},
                    {id: "srv-03", host: "ftp.files.com", protocol: "FTP", name: "Client-Assets", user: "anonymous", status: "Inactive"}
                ]
                
                delegate: Rectangle {
                    width: parent.width; height: 60; radius: 8
                    color: itemHover.hovered ? "#252525" : "#1e1e1e"; border.color: "#333"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        
                        Rectangle { width: 40; height: 40; radius: 20; color: "#0d0d0d"
                            Label { anchors.centerIn: parent; text: modelData.protocol === "SMB" ? "🏠" : "☁️"; font.pixelSize: 20 } }
                        
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.name + " (" + modelData.host + ")"; color: "white"; font.bold: true; font.pixelSize: 13 }
                            Label { text: modelData.protocol + " • user: " + modelData.user; color: "#888"; font.pixelSize: 10 }
                        }
                        
                        Rectangle { width: 70; height: 24; radius: 12; color: modelData.status === "Active" ? "#1a3a1a" : "#333"
                            Label { anchors.centerIn: parent; text: modelData.status; color: modelData.status === "Active" ? "#4caf50" : "#666"; font.pixelSize: 9; font.bold: true } }
                        
                        Button { text: modelData.status === "Active" ? "Disconnect" : "Connect"
                            onClicked: { /* connection logic */ }
                            background: Rectangle { color: parent.text === "Connect" ? "#0078d4" : "#3a1a1a"; radius: 4 }
                            contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 10 } }
                        
                        Button { text: "⚙️"; flat: true; contentItem: Label { text: parent.text; color: "#666" } }
                    }
                    HoverHandler { id: itemHover }
                }
            }
        }

        // ── New Connection Popup ──────────────────────────────────────────
        Popup {
            id: addConnPopup
            parent: Overlay.overlay; x: (parent.width - width)/2; y: 100; width: 450; height: 450
            background: Rectangle { color: "#1e1e1e"; radius: 8; border.color: "#333" }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 25; spacing: 12
                Label { text: "Add Remote Location"; color: "white"; font.bold: true; font.pixelSize: 18 }
                
                GridLayout { columns: 2; rowSpacing: 10; columnSpacing: 10
                    Label { text: "Protocol:"; color: "#888" }
                    ComboBox { model: ["SFTP (SSH)", "FTP", "SMB (Samba)", "WebDAV"]; Layout.fillWidth: true }
                    
                    Label { text: "Host:"; color: "#888" }
                    TextField { placeholderText: "example.com or IP"; Layout.fillWidth: true }
                    
                    Label { text: "Port:"; color: "#888" }
                    TextField { text: "22"; Layout.fillWidth: true }
                    
                    Label { text: "Username:"; color: "#888" }
                    TextField { placeholderText: "root"; Layout.fillWidth: true }
                    
                    Label { text: "Password:"; color: "#888" }
                    TextField { echoMode: TextInput.Password; Layout.fillWidth: true }
                    
                    Label { text: "Remote Path:"; color: "#888" }
                    TextField { text: "/"; Layout.fillWidth: true }
                }
                
                Item { Layout.fillHeight: true }
                RowLayout {
                    spacing: 10; Layout.alignment: Qt.AlignRight
                    Button { text: "Cancel"; flat: true; onClicked: addConnPopup.close() }
                    Button { text: "Test & Save"; background: Rectangle { color: "#0078d4"; radius: 6 }; contentItem: Label { text: parent.text; color: "white" }; onClicked: addConnPopup.close() }
                }
            }
        }
    }
}
