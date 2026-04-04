import QtQuick 2.15
import QtQuick.Layouts 1.15

/// GamificationPanel.qml — Progress and Reward system for file organization.
/// Tracks space reclaimed, files organized, and provides a Level/XP system.
/// "Gamified Cleanup" from IDEAS.md.

Rectangle {
    id: gamePanel
    color: "#0f0f0f"
    radius: 8

    property int userLevel: 14
    property real currentXp: 2450
    property real maxXp: 5000
    property real totalSpaceReclaimedGb: 142.5
    property int filesOrganizedCount: 8432

    component PanelButton: Rectangle {
        id: panelButton
        property alias text: buttonText.text
        property color textColor: "white"
        property color fillColor: "#0078d4"
        property color borderLineColor: "transparent"
        property bool flat: false
        signal clicked

        radius: 6
        height: 32
        color: flat ? (buttonHover.hovered ? "#22ffffff" : "transparent") : fillColor
        border.color: flat ? "transparent" : borderLineColor
        implicitWidth: buttonText.implicitWidth + 24

        Text {
            id: buttonText
            anchors.centerIn: parent
            color: panelButton.textColor
            font.bold: true
            font.pixelSize: 12
        }

        HoverHandler { id: buttonHover }
        MouseArea {
            anchors.fill: parent
            onClicked: panelButton.clicked()
        }
    }

    component SlimProgressBar: Item {
        id: progressRoot
        property real value: 0.0
        property color fillColor: "#0078d4"
        property int barHeight: 8

        implicitWidth: 100
        implicitHeight: barHeight

        Rectangle {
            anchors.fill: parent
            radius: progressRoot.barHeight / 2
            color: "#1a1a1a"
        }

        Rectangle {
            width: parent.width * Math.max(0, Math.min(1, progressRoot.value))
            height: parent.height
            radius: progressRoot.barHeight / 2
            color: progressRoot.fillColor
        }
    }

    component StatCard: Rectangle {
        id: statCard
        property alias title: statTitle.text
        property alias value: statValue.text
        property alias subtitle: statSubtitle.text
        property color accentColor: "white"

        Layout.fillWidth: true
        height: 120
        radius: 12
        color: "#1a1a1a"
        border.color: "#333"

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 4

            Text {
                id: statTitle
                color: "#888"
                font.pixelSize: 12
            }

            Text {
                id: statValue
                color: statCard.accentColor
                font.pixelSize: 32
                font.bold: true
            }

            Text {
                id: statSubtitle
                color: statCard.accentColor
                font.pixelSize: 10
                opacity: 0.7
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 25

        // ── User Header ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            // Avatar Circle
            Rectangle {
                width: 80
                height: 80
                radius: 40
                color: "#1e1e1e"
                border.color: "#0078d4"
                border.width: 2

                Text {
                    anchors.centerIn: parent
                    text: "👤"
                    font.pixelSize: 40
                    color: "white"
                }
            }

            ColumnLayout {
                spacing: 4

                Text {
                    text: "Level " + gamePanel.userLevel + " Data Custodian"
                    color: "white"
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    text: "Rank: Platinum Organizer"
                    color: "#0078d4"
                    font.pixelSize: 14
                    font.bold: true
                }
                
                // XP Bar
                ColumnLayout {
                    spacing: 4
                    Layout.preferredWidth: 300

                    RowLayout {
                        Text {
                            text: "XP: " + gamePanel.currentXp + " / " + gamePanel.maxXp
                            color: "#888"
                            font.pixelSize: 10
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: "49%"
                            color: "#888"
                            font.pixelSize: 10
                        }
                    }

                    SlimProgressBar {
                        Layout.fillWidth: true
                        value: gamePanel.currentXp / gamePanel.maxXp
                        fillColor: "#0078d4"
                        barHeight: 8
                    }
                }
            }
        }

        // ── Stats Cards ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            StatCard {
                title: "Space Reclaimed"
                value: gamePanel.totalSpaceReclaimedGb + " GB"
                subtitle: "Top 5% of users"
                accentColor: "#4caf50"
            }

            StatCard {
                title: "Files Organized"
                value: gamePanel.filesOrganizedCount.toLocaleString()
                subtitle: "Digital hoarder status: Cured"
                accentColor: "#ffaa00"
            }

            StatCard {
                title: "Current Multiplier"
                value: "2.5x"
                subtitle: "Combo: 3 days streak"
                accentColor: "#0078d4"
            }
        }

        // ── Achievements ──────────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: "Unlocked Achievements"
                color: "#aaa"
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 6
                color: "#161616"
                border.color: "#333"

                Flow {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15
                    
                    component Achievement: Rectangle {
                        id: achievementRoot
                        width: 100
                        height: 100
                        radius: 50
                        color: unlocked ? "#1a3a5a" : "#111"
                        border.color: unlocked ? "#0078d4" : "#222"
                        border.width: 2

                        property bool unlocked: true
                        property string icon: "🏆"
                        property string title: ""

                        Column {
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: achievementRoot.icon
                                font.pixelSize: 32
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "white"
                            }

                            Text {
                                text: achievementRoot.title
                                color: achievementRoot.unlocked ? "white" : "#444"
                                font.pixelSize: 9
                                horizontalAlignment: Text.AlignHCenter
                                width: 80
                                wrapMode: Text.WordWrap
                            }
                        }

                        Rectangle {
                            visible: achHover.hovered
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.top
                            anchors.bottomMargin: 8
                            radius: 4
                            color: "#dd111111"
                            border.color: "#333"
                            width: tooltipText.implicitWidth + 12
                            height: tooltipText.implicitHeight + 8
                            z: 10

                            Text {
                                id: tooltipText
                                anchors.centerIn: parent
                                text: achievementRoot.title + (achievementRoot.unlocked ? " (Unlocked)" : " (Locked)")
                                color: "white"
                                font.pixelSize: 10
                            }
                        }

                        HoverHandler { id: achHover }
                    }

                    Achievement { title: "Deduplicator"; icon: "👯" }
                    Achievement { title: "Converter King"; icon: "🔄" }
                    Achievement { title: "Privacy Shield"; icon: "🛡️" }
                    Achievement { title: "Clean Sweep"; icon: "🧹" }
                    Achievement { title: "Vault Master"; icon: "🔒" }
                    Achievement { title: "Reflink Ninja"; icon: "⚡" }
                    Achievement { title: "Metadata Medic"; unlocked: false; icon: "🏷️" }
                    Achievement { title: "Cloud Climber"; unlocked: false; icon: "☁️" }
                }
            }
        }

        // ── Daily Quest ───────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 80
            radius: 10
            color: "#0078d422"
            border.color: "#0078d4"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Text {
                    text: "🎯 Daily Quest:"
                    color: "white"
                    font.bold: true
                }

                Text {
                    text: "Organize 50 photos from /Downloads"
                    color: "#ccc"
                    Layout.fillWidth: true
                }

                Text {
                    text: "Reward: +500 XP"
                    color: "#0078d4"
                    font.bold: true
                }

                PanelButton {
                    text: "Start"
                    fillColor: "#0078d4"
                    textColor: "white"
                }
            }
        }
    }
}
