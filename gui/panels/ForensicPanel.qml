import QtQuick 2.15
import QtQuick.Layouts 1.15

/// ForensicPanel.qml — High-integrity Audit Ledger and Verification UI.
/// Monitors the immutable record of all file operations.
/// Features tamper-detection, chain-of-custody, and forensic search.

Rectangle {
    id: forensicPanel
    color: "#0a0a0a"
    radius: 8

    property bool isVerifying: false
    property real integrityScore: 1.0
    property var auditEntries: []

    component PanelButton: Rectangle {
        id: panelButton
        property alias text: buttonText.text
        property color textColor: "white"
        property color fillColor: "#0078d4"
        property color borderLineColor: "transparent"
        property bool flat: false
        signal clicked

        radius: 6
        height: 36
        color: flat ? (buttonHover.hovered ? "#22ffffff" : "transparent") : fillColor
        border.color: flat ? "transparent" : borderLineColor
        implicitWidth: buttonText.implicitWidth + 28

        Text {
            id: buttonText
            anchors.centerIn: parent
            color: panelButton.textColor
            font.bold: true
            font.pixelSize: 13
        }

        HoverHandler { id: buttonHover }
        MouseArea {
            anchors.fill: parent
            onClicked: panelButton.clicked()
        }
    }

    component StatCard: Rectangle {
        id: statCard
        property alias title: statTitle.text
        property alias value: statValue.text
        property color valueColor: "white"

        Layout.fillWidth: true
        height: 80
        radius: 10
        color: "#161616"
        border.color: "#333"

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 4

            Text {
                id: statTitle
                color: "#888"
                font.pixelSize: 11
            }

            Text {
                id: statValue
                color: statCard.valueColor
                font.bold: true
                font.pixelSize: 16
            }
        }
    }

    function integrityFillColor() {
        if (integrityScore === 1.0) {
            return "#1a3a1a"
        }
        return "#3a1a1a"
    }

    function integrityTextColor() {
        if (integrityScore === 1.0) {
            return "#4caf50"
        }
        return "#ff4444"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15

            Text {
                text: "🛡️ Forensic Audit Ledger"
                font.pixelSize: 22
                font.bold: true
                color: "white"
            }

            Rectangle {
                width: 90
                height: 24
                radius: 12
                color: forensicPanel.integrityFillColor()

                Text {
                    anchors.centerIn: parent
                    text: forensicPanel.integrityScore === 1.0 ? "VERIFIED" : "COMPROMISED"
                    color: forensicPanel.integrityTextColor()
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Item { Layout.fillWidth: true }
            
            PanelButton {
                text: forensicPanel.isVerifying ? "⏳ Verifying Hashes..." : "🔍 Run Integrity Check"
                fillColor: "#0078d4"
                onClicked: forensicPanel.isVerifying = !forensicPanel.isVerifying
            }
        }

        // ── Ledger Info ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20

            StatCard {
                title: "Ledger State"
                value: "Immutable Chain"
                valueColor: "white"
            }

            StatCard {
                title: "Total Entries"
                value: "12,842"
                valueColor: "white"
            }

            StatCard {
                title: "System ID"
                value: "NODE-84-F2-C1"
                valueColor: "#0078d4"
            }
        }

        // ── Audit Table ───────────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: "Chain of Custody"
                color: "#aaa"
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 6
                color: "#111"
                border.color: "#333"

                ListView {
                    id: auditList
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    spacing: 2
                    model: [
                        {op: "MOVE", src: "/Downloads/invoice.pdf", dst: "/Vault/invoice.pdf", user: "system", hash: "a3f2...c1b8", time: "14:32:05"},
                        {op: "DELETE", src: "/Temp/cache_1.tmp", dst: "", user: "robert", hash: "9e11...49e1", time: "14:30:12"},
                        {op: "RENAME", src: "IMG_01.JPG", dst: "Tahoe_01.JPG", user: "robert", hash: "338a...ba09", time: "14:28:44"},
                        {op: "CONVERT", src: "vid.mov", dst: "vid.mp4", user: "system", hash: "4571...3184", time: "14:25:33"}
                    ]
                    
                    header: Rectangle {
                        width: auditList.width
                        height: 30
                        color: "#1a1a1a"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10

                            Text { text: "TIME"; color: "#666"; font.pixelSize: 10; width: 80 }
                            Text { text: "OP"; color: "#666"; font.pixelSize: 10; width: 80 }
                            Text { text: "SOURCE"; color: "#666"; font.pixelSize: 10; Layout.fillWidth: true }
                            Text { text: "TARGET"; color: "#666"; font.pixelSize: 10; Layout.fillWidth: true }
                            Text { text: "USER"; color: "#666"; font.pixelSize: 10; width: 80 }
                            Text { text: "HASH (HMAC)"; color: "#666"; font.pixelSize: 10; width: 100 }
                        }
                    }

                    delegate: Rectangle {
                        width: auditList.width
                        height: 32
                        color: index % 2 === 0 ? "#0d0d0d" : "#111"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10

                            Text {
                                text: modelData.time
                                color: "#888"
                                font.pixelSize: 11
                                width: 80
                            }

                            Rectangle {
                                width: 70
                                height: 20
                                radius: 4
                                color: "#22ffffff"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.op
                                    color: "white"
                                    font.pixelSize: 9
                                    font.bold: true
                                }
                            }

                            Text {
                                text: modelData.src
                                color: "#ccc"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                elide: Text.ElideMiddle
                            }

                            Text {
                                text: modelData.dst || "—"
                                color: "#aaa"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                elide: Text.ElideMiddle
                            }

                            Text {
                                text: modelData.user
                                color: "#888"
                                font.pixelSize: 11
                                width: 80
                            }

                            Text {
                                text: modelData.hash
                                color: "#4caf50"
                                font.pixelSize: 10
                                font.family: "Consolas"
                                width: 100
                            }
                        }
                    }
                }
            }
        }

        // ── Controls Footer ───────────────────────────────────────────────
        RowLayout {
            spacing: 15

            PanelButton {
                text: "📋 Export Legal Report"
                flat: true
                textColor: "#888"
            }

            PanelButton {
                text: "💾 Save Signed Ledger"
                flat: true
                textColor: "#888"
            }

            Item { Layout.fillWidth: true }

            PanelButton {
                text: "⚠️ Clear History"
                fillColor: "#3a1a1a"
                textColor: "#ff4444"
            }
        }
    }
}
