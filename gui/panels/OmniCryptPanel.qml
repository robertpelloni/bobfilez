import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniCryptPanel.qml — Advanced Cryptography and Steganography Suite.
/// High-security panel for AES-256-GCM encryption and LSB steganography.
/// Part of the "Privacy Sentinel" suite.

Rectangle {
    id: cryptPanel
    color: "#050505"; radius: 8

    property int activeTab: 0 // 0=Encryption, 1=Steganography
    property bool isProcessing: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🔏 OmniCrypt Suite"; font.pixelSize: 22; font.bold: true; color: "#ff4444" }
            Label { text: "Military-Grade Encryption & Plausible Deniability"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: 160; height: 24; radius: 12; color: "#3a1a1a"; border.color: "#ff4444"
                Label { anchors.centerIn: parent; text: "libsodium / XChaCha20"; color: "#ff4444"; font.pixelSize: 10; font.bold: true; font.family: "Consolas" }
            }
        }

        // ── Tabs ──────────────────────────────────────────────────────────
        RowLayout {
            spacing: 5
            Repeater {
                model: [
                    {name: "🔒 Standard Encryption", idx: 0},
                    {name: "🖼️ Steganography (Hide in Image)", idx: 1}
                ]
                Button {
                    text: modelData.name
                    background: Rectangle { color: cryptPanel.activeTab === modelData.idx ? "#ff4444" : "#1a1a1a"; radius: 4 }
                    contentItem: Label { text: parent.text; color: cryptPanel.activeTab === modelData.idx ? "white" : "#aaa"; font.bold: true; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter }
                    onClicked: cryptPanel.activeTab = modelData.idx
                    Layout.preferredWidth: 250; height: 35
                }
            }
        }

        // ── Main Content Area ─────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#111"; radius: 6; border.color: "#333"
            clip: true

            // 1. Standard Encryption (AES/ChaCha)
            ColumnLayout {
                visible: cryptPanel.activeTab === 0
                anchors.fill: parent; anchors.margins: 20; spacing: 15
                
                Label { text: "File Encryption (XChaCha20-Poly1305)"; color: "#aaa"; font.bold: true; font.pixelSize: 14 }
                
                GridLayout {
                    columns: 2; columnSpacing: 15; rowSpacing: 10; Layout.fillWidth: true
                    Label { text: "Target File:"; color: "#888" }
                    RowLayout {
                        TextInput { text: "C:/Documents/secret_passwords.txt"; color: "white"; font.family: "Consolas"; Layout.fillWidth: true; readOnly: true; background: Rectangle { color: "#252525"; radius: 4 } }
                        Button { text: "Browse"; flat: true }
                    }
                    
                    Label { text: "Password:"; color: "#888" }
                    TextInput { placeholderText: "Master Password"; echoMode: TextInput.Password; color: "white"; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 } }
                }

                Item { Layout.fillHeight: true }
                RowLayout {
                    spacing: 15
                    Button { text: "🔓 Decrypt"; background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }; contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true } }
                    Button { text: "🔒 Encrypt"; background: Rectangle { color: "#3a1a1a"; radius: 6; border.color: "#ff4444" }; contentItem: Label { text: parent.text; color: "#ff4444"; font.bold: true } }
                }
            }

            // 2. Steganography (Hide in Image)
            ColumnLayout {
                visible: cryptPanel.activeTab === 1
                anchors.fill: parent; anchors.margins: 20; spacing: 15

                Label { text: "LSB Steganography (OpenCV)"; color: "#aaa"; font.bold: true; font.pixelSize: 14 }
                Label { text: "Hide sensitive files completely inside innocent images. The payload is encrypted with XChaCha20-Poly1305 before being embedded into the Least Significant Bits of the image pixels."; color: "#666"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                
                RowLayout {
                    Layout.fillWidth: true; spacing: 20
                    
                    // Carrier Image
                    GroupBox {
                        label: Label { text: "Carrier Image (The Disguise)"; color: "#aaa"; font.bold: true }
                        Layout.fillWidth: true; Layout.preferredHeight: 180
                        background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                        
                        ColumnLayout { anchors.fill: parent; spacing: 5
                            Rectangle {
                                Layout.fillWidth: true; Layout.fillHeight: true; color: "black"; radius: 4
                                Image { anchors.fill: parent; anchors.margins: 4; source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectCrop }
                                Label { anchors.centerIn: parent; text: "cat_photo.jpg"; color: "white"; style: Text.Outline; styleColor: "black" }
                            }
                            Label { text: "Stego Capacity: 5.2 MB"; color: "#4caf50"; font.pixelSize: 10; font.bold: true }
                        }
                    }

                    // Payload File
                    GroupBox {
                        label: Label { text: "Payload (The Secret)"; color: "#aaa"; font.bold: true }
                        Layout.fillWidth: true; Layout.preferredHeight: 180
                        background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                        
                        ColumnLayout { anchors.fill: parent; spacing: 5
                            Rectangle {
                                Layout.fillWidth: true; Layout.fillHeight: true; color: "#252525"; radius: 4; border.color: "#444"; border.width: 2; border.style: Border.Dashed
                                Label { anchors.centerIn: parent; text: "📄 Drop Secret File Here"; color: "#888" }
                            }
                            Label { text: "Payload Size: 1.4 MB (26% Capacity)"; color: "#ffaa00"; font.pixelSize: 10 }
                        }
                    }
                }

                RowLayout {
                    Label { text: "Encryption Password:"; color: "#888" }
                    TextInput { placeholderText: "Master Password"; echoMode: TextInput.Password; color: "white"; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 } }
                }

                Item { Layout.fillHeight: true }
                RowLayout {
                    spacing: 15
                    Button { text: "🔓 Extract Payload"; background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }; contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true } }
                    Item { Layout.fillWidth: true }
                    Button { text: "🔏 Encode & Hide Payload"; background: Rectangle { color: "#3a1a1a"; radius: 6; border.color: "#ff4444" }; contentItem: Label { text: parent.text; color: "#ff4444"; font.bold: true } }
                }
            }
        }
    }
}
