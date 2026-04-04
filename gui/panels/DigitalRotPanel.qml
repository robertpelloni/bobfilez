import QtQuick 2.15
import QtQuick.Layouts 1.15

/// DigitalRotPanel.qml — Autonomous "Digital Rot" Cleanup Agent.
/// Suggests files to prune based on age, redundancy, and file type patterns.
/// Part of the "Data Custodian" suite.

Rectangle {
    id: rotPanel
    color: "#0f0f0f"
    radius: 8

    property var suggestions: [] // [{path, reason, confidence, size}]
    property bool isScanning: false
    property real reclaimableBytes: 0

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
                font.pixelSize: 28
                font.bold: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15

            Text {
                text: "🧹 Digital Rot Agent"
                font.pixelSize: 22
                font.bold: true
                color: "white"
            }

            Text {
                text: "AI-Powered System Pruning"
                color: "#888"
                font.pixelSize: 14
            }

            Item { Layout.fillWidth: true }

            PanelButton {
                text: rotPanel.isScanning ? "⏳ Analyzing..." : "🔍 Scan for Rot"
                fillColor: "#0078d4"
                onClicked: rotPanel.isScanning = !rotPanel.isScanning
            }
        }

        // ── Stats Row ──────────────────────────────────────────────────────
        RowLayout {
            spacing: 20

            StatCard {
                title: "Reclaimable Space"
                value: (rotPanel.reclaimableBytes / (1024 * 1024 * 1024)).toFixed(2) + " GB"
                valueColor: "#4caf50"
            }

            StatCard {
                title: "Risk Files Found"
                value: rotPanel.suggestions.length.toString()
                valueColor: "#ffaa00"
            }

            StatCard {
                title: "System Health Score"
                value: "84 / 100"
                valueColor: "#0078d4"
            }
        }

        // ── Suggestions List ──────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: "Pruning Suggestions"
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
                    spacing: 5
                    model: [
                        {path: "/Downloads/chrome_old_setup.exe", reason: "Obsolete Installer", confidence: 0.95, size: "124 MB"},
                        {path: "/Temp/cache_293.tmp", reason: "Temporary File", confidence: 1.0, size: "450 MB"},
                        {path: "/Documents/Project_Archive_2019.zip", reason: "Forgotten Data", confidence: 0.72, size: "2.4 GB"},
                        {path: "/Pictures/Blurry_001.jpg", reason: "AI: Blurry Photo", confidence: 0.85, size: "4.5 MB"}
                    ]

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 60
                        radius: 6
                        color: itemHover.hovered ? "#252525" : "#1e1e1e"
                        border.color: "#333"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 15

                            Text {
                                text: "🗑️"
                                font.pixelSize: 18
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
                                    Layout.fillWidth: true
                                }

                                RowLayout {
                                    Text {
                                        text: modelData.reason
                                        color: "#0078d4"
                                        font.pixelSize: 11
                                        font.bold: true
                                    }

                                    Text {
                                        text: " • "
                                        color: "#444"
                                    }

                                    Text {
                                        text: "Confidence: " + Math.round(modelData.confidence * 100) + "%"
                                        color: "#888"
                                        font.pixelSize: 10
                                    }
                                }
                            }

                            Text {
                                text: modelData.size
                                color: "#aaa"
                                font.pixelSize: 12
                                font.bold: true
                            }

                            PanelButton {
                                text: "Remove"
                                height: 30
                                fillColor: "#3a1a1a"
                                borderLineColor: "#ff4444"
                                textColor: "#ff4444"
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
                text: "✨ Smart Purge (Remove >90% Confidence)"
                fillColor: "#1a4a1a"
                borderLineColor: "#4caf50"
                textColor: "#4caf50"
            }

            Item { Layout.fillWidth: true }

            PanelButton {
                text: "⚙️ Pruning Settings"
                flat: true
                textColor: "#888"
            }
        }
    }
}
