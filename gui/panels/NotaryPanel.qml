import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// NotaryPanel.qml — Digital Notary and Cryptographic Signing UI.
/// Sign files with Ed25519 or GPG to ensure authenticity.
/// Part of the "Forensic Integrity" suite.

Rectangle {
    id: notaryPanel
    color: "#0f0f0f"; radius: 8

    property string selectedFilePath: ""
    property string currentSignature: ""
    property bool isSigning: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🔏 Digital Notary"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Cryptographic File Signing"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
        }

        // ── Main Content ──
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 30

            // Left: File Selector & Key Info
            ColumnLayout {
                Layout.preferredWidth: 400; spacing: 20
                
                GroupBox {
                    label: Label { text: "Active Identity"; color: "#aaa"; font.bold: true }
                    Layout.fillWidth: true
                    background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 8
                        Label { text: "User: Robert Pelloni <robert@bob.com>"; color: "white"; font.pixelSize: 13 }
                        Label { text: "Key ID: 0x84F2C1B8 (Ed25519)"; color: "#0078d4"; font.pixelSize: 11; font.family: "Consolas" }
                        Button { text: "Change Key"; flat: true; contentItem: Label { text: parent.text; color: "#666" } }
                    }
                }

                GroupBox {
                    label: Label { text: "File to Sign/Verify"; color: "#aaa"; font.bold: true }
                    Layout.fillWidth: true
                    background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 12
                        Label { text: notaryPanel.selectedFilePath ? notaryPanel.selectedFilePath.split("/").pop() : "No file selected"; color: "white"; font.pixelSize: 14 }
                        Button { text: "Select File..."; onClicked: {} }
                    }
                }
                
                Item { Layout.fillHeight: true }
            }

            // Right: Signature Panel
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; color: "#0d0d0d"; radius: 10; border.color: "#333"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 15
                    Label { text: "SIGNATURE DATA"; color: "#666"; font.bold: true; font.pixelSize: 11 }
                    
                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; color: "black"; radius: 6
                        ScrollView {
                            anchors.fill: parent; anchors.margins: 10
                            TextArea {
                                text: "-----BEGIN BOBFILEZ SIGNATURE-----\n" +
                                      "Version: 4.2.0\n" +
                                      "Hash: SHA-512\n\n" +
                                      "iQEzBAEBCAAdFiEE6Fv...base64_data...Xv5K\n" +
                                      "-----END BOBFILEZ SIGNATURE-----"
                                color: "#00ff00"; font.family: "Consolas"; font.pixelSize: 11; readOnly: true
                            }
                        }
                    }

                    RowLayout {
                        spacing: 15
                        Button {
                            text: "🔏 Sign File"
                            Layout.fillWidth: true; height: 40
                            background: Rectangle { color: "#0078d4"; radius: 6 }
                            contentItem: Label { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                        }
                        Button {
                            text: "🔍 Verify Signature"
                            Layout.fillWidth: true; height: 40
                            background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                            contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                        }
                    }
                }
            }
        }

        // ── History Footer ────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Recent Notarizations"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; height: 150
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; model: 3
                delegate: Rectangle {
                    width: parent.width; height: 30; color: index % 2 === 0 ? "#1a1a1a" : "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 10
                        Label { text: "✓"; color: "#4caf50" }
                        Label { text: "contract_signed.pdf"; color: "#ccc"; font.pixelSize: 11; Layout.fillWidth: true }
                        Label { text: "2024-07-12 14:32"; color: "#666"; font.pixelSize: 10 }
                        Label { text: "verified"; color: "#0078d4"; font.pixelSize: 9; font.bold: true }
                    }
                }
            }
        }
    }
}
