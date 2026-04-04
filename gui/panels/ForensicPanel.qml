import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// ForensicPanel.qml — High-integrity Audit Ledger and Verification UI.
/// Monitors the immutable record of all file operations.
/// Features tamper-detection, chain-of-custody, and forensic search.

Rectangle {
    id: forensicPanel
    color: "#0a0a0a"; radius: 8

    property bool isVerifying: false
    property real integrityScore: 1.0
    property var auditEntries: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🛡️ Forensic Audit Ledger"; font.pixelSize: 22; font.bold: true; color: "white" }
            Rectangle { width: 90; height: 24; radius: 12; color: forensicPanel.integrityScore === 1.0 ? "#1a3a1a" : "#3a1a1a"
                Label { anchors.centerIn: parent; text: forensicPanel.integrityScore === 1.0 ? "VERIFIED" : "COMPROMISED"; color: forensicPanel.integrityScore === 1.0 ? "#4caf50" : "#ff4444"; font.pixelSize: 10; font.bold: true } }
            Item { Layout.fillWidth: true }
            
            Button {
                text: forensicPanel.isVerifying ? "⏳ Verifying Hashes..." : "🔍 Run Integrity Check"
                background: Rectangle { color: "#0078d4"; radius: 6 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                onClicked: forensicPanel.isVerifying = !forensicPanel.isVerifying
            }
        }

        // ── Ledger Info ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 10; color: "#161616"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Ledger State"; color: "#888"; font.pixelSize: 11 }
                    Label { text: "Immutable Chain"; color: "white"; font.bold: true; font.pixelSize: 16 }
                }
            }
            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 10; color: "#161616"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Total Entries"; color: "#888"; font.pixelSize: 11 }
                    Label { text: "12,842"; color: "white"; font.bold: true; font.pixelSize: 16 }
                }
            }
            Rectangle {
                Layout.fillWidth: true; height: 80; radius: 10; color: "#161616"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "System ID"; color: "#888"; font.pixelSize: 11 }
                    Label { text: "NODE-84-F2-C1"; color: "#0078d4"; font.bold: true; font.pixelSize: 16 }
                }
            }
        }

        // ── Audit Table ───────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Chain of Custody"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#111"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                id: auditList
                anchors.fill: parent; clip: true; spacing: 2
                ScrollBar.vertical: ScrollBar {}
                model: [
                    {op: "MOVE", src: "/Downloads/invoice.pdf", dst: "/Vault/invoice.pdf", user: "system", hash: "a3f2...c1b8", time: "14:32:05"},
                    {op: "DELETE", src: "/Temp/cache_1.tmp", dst: "", user: "robert", hash: "9e11...49e1", time: "14:30:12"},
                    {op: "RENAME", src: "IMG_01.JPG", dst: "Tahoe_01.JPG", user: "robert", hash: "338a...ba09", time: "14:28:44"},
                    {op: "CONVERT", src: "vid.mov", dst: "vid.mp4", user: "system", hash: "4571...3184", time: "14:25:33"}
                ]
                
                header: Rectangle {
                    width: auditList.width; height: 30; color: "#1a1a1a"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
                        Label { text: "TIME"; color: "#666"; font.pixelSize: 10; width: 80 }
                        Label { text: "OP"; color: "#666"; font.pixelSize: 10; width: 80 }
                        Label { text: "SOURCE"; color: "#666"; font.pixelSize: 10; Layout.fillWidth: true }
                        Label { text: "TARGET"; color: "#666"; font.pixelSize: 10; Layout.fillWidth: true }
                        Label { text: "USER"; color: "#666"; font.pixelSize: 10; width: 80 }
                        Label { text: "HASH (HMAC)"; color: "#666"; font.pixelSize: 10; width: 100 }
                    }
                }

                delegate: Rectangle {
                    width: auditList.width; height: 32; color: index % 2 === 0 ? "#0d0d0d" : "#111"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
                        Label { text: modelData.time; color: "#888"; font.pixelSize: 11; width: 80 }
                        Rectangle { width: 70; height: 20; radius: 4; color: "#22ffffff"
                            Label { anchors.centerIn: parent; text: modelData.op; color: "white"; font.pixelSize: 9; font.bold: true } }
                        Label { text: modelData.src; color: "#ccc"; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideMiddle }
                        Label { text: modelData.dst || "—"; color: "#aaa"; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideMiddle }
                        Label { text: modelData.user; color: "#888"; font.pixelSize: 11; width: 80 }
                        Label { text: modelData.hash; color: "#4caf50"; font.pixelSize: 10; font.family: "Consolas"; width: 100 }
                    }
                }
            }
        }

        // ── Controls Footer ───────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Button { text: "📋 Export Legal Report"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
            Button { text: "💾 Save Signed Ledger"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
            Item { Layout.fillWidth: true }
            Button {
                text: "⚠️ Clear History"; background: Rectangle { color: "#3a1a1a"; radius: 6 }
                contentItem: Label { text: parent.text; color: "#ff4444"; font.bold: true }
            }
        }
    }
}
