import QtQuick 2.15
import QtQuick.Layouts 1.15

/// RecoveryPanel.qml — Forensic Data Recovery and Self-Healing UI.
/// Detect bit-rot, undelete files, and restore from Swarm/Cloud mirrors.
/// "The Data Resurrection" suite.

Rectangle {
    id: recoveryPanel
    color: "#0f0f0f"
    radius: 8

    property bool isScrubbing: false
    property int corruptionCount: 2
    property var reports: []

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
        height: 100
        radius: 10
        color: "#1a1a1a"
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
                font.pixelSize: 24
            }
        }
    }

    function corruptionColor() {
        if (corruptionCount > 0) {
            return "#ff4444"
        }
        return "#4caf50"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15

            Text {
                text: "🩹 Data Recovery & Healing"
                font.pixelSize: 22
                font.bold: true
                color: "white"
            }

            Text {
                text: "Bit-rot Protection"
                color: "#888"
                font.pixelSize: 14
            }

            Item { Layout.fillWidth: true }
            
            PanelButton {
                text: recoveryPanel.isScrubbing ? "⏳ Scrubbing Disk..." : "🔍 Start Data Scrub"
                fillColor: "#0078d4"
                onClicked: recoveryPanel.isScrubbing = !recoveryPanel.isScrubbing
            }
        }

        // ── Health Dashboard ──────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            StatCard {
                title: "Files Verified"
                value: "124,532"
                valueColor: "white"
            }

            StatCard {
                title: "Corruptions Detected"
                value: recoveryPanel.corruptionCount.toString()
                valueColor: recoveryPanel.corruptionColor()
            }

            StatCard {
                title: "Redundancy Level"
                value: "3x Mirror"
                valueColor: "#0078d4"
            }
        }

        // ── Corruption Reports List ────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: "Integrity Exceptions"
                color: "#aaa"
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 6
                color: "#161616"
                border.color: "#333"

                ListView {
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    spacing: 8
                    model: [
                        {path: "/Archive/wedding_01.raw", error: "Bit-rot: Checksum Mismatch", source: "AWS S3", canHeal: true},
                        {path: "/Work/Report_Final.pdf", error: "Silent Data Corruption", source: "Swarm: Laptop-X1", canHeal: true},
                        {path: "/Users/robert/key.txt", error: "File Missing (Deleted)", source: "Nexus Vault", canHeal: true}
                    ]
                    
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 70
                        radius: 8
                        color: itemHover.hovered ? "#252525" : "#1e1e1e"
                        border.color: "#333"
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 15

                            Text {
                                text: "⚠️"
                                font.pixelSize: 24
                                color: "white"
                            }
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: modelData.path
                                    color: "white"
                                    font.pixelSize: 13
                                    elide: Text.ElideMiddle
                                }

                                Text {
                                    text: modelData.error
                                    color: "#ff4444"
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                            
                            ColumnLayout {
                                spacing: 4

                                Text {
                                    text: "Source: " + modelData.source
                                    color: "#888"
                                    font.pixelSize: 10
                                }

                                PanelButton {
                                    text: "Heal File"
                                    height: 30
                                    fillColor: "#1a3a1a"
                                    borderLineColor: "#4caf50"
                                    textColor: "#4caf50"
                                }
                            }
                        }

                        HoverHandler { id: itemHover }
                    }
                }
            }
        }

        // ── Controls Footer ───────────────────────────────────────────────
        RowLayout {
            spacing: 15

            PanelButton {
                text: "♻️ Undelete Browser"
                flat: true
                textColor: "#888"
            }

            PanelButton {
                text: "📅 Scrub Schedule"
                flat: true
                textColor: "#888"
            }

            Item { Layout.fillWidth: true }

            PanelButton {
                text: "✨ Heal All Corruptions"
                fillColor: "#1a4a1a"
                borderLineColor: "#4caf50"
                textColor: "#4caf50"
            }
        }
    }
}
