import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// GamificationPanel.qml — Progress and Reward system for file organization.
/// Tracks space reclaimed, files organized, and provides a Level/XP system.
/// "Gamified Cleanup" from IDEAS.md.

Rectangle {
    id: gamePanel
    color: "#0f0f0f"; radius: 8

    property int userLevel: 14
    property real currentXp: 2450
    property real maxXp: 5000
    property real totalSpaceReclaimedGb: 142.5
    property int filesOrganizedCount: 8432

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 25

        // ── User Header ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            // Avatar Circle
            Rectangle {
                width: 80; height: 80; radius: 40
                color: "#1e1e1e"; border.color: "#0078d4"; border.width: 2
                Label { anchors.centerIn: parent; text: "👤"; font.pixelSize: 40 }
            }

            ColumnLayout {
                spacing: 4
                Label { text: "Level " + gamePanel.userLevel + " Data Custodian"; color: "white"; font.pixelSize: 22; font.bold: true }
                Label { text: "Rank: Platinum Organizer"; color: "#0078d4"; font.pixelSize: 14; font.bold: true }
                
                // XP Bar
                ColumnLayout {
                    spacing: 4; Layout.preferredWidth: 300
                    RowLayout {
                        Label { text: "XP: " + gamePanel.currentXp + " / " + gamePanel.maxXp; color: "#888"; font.pixelSize: 10 }
                        Item { Layout.fillWidth: true }
                        Label { text: "49%"; color: "#888"; font.pixelSize: 10 }
                    }
                    ProgressBar {
                        Layout.fillWidth: true; value: gamePanel.currentXp / gamePanel.maxXp
                        background: Rectangle { color: "#1a1a1a"; radius: 4; implicitHeight: 8 }
                        contentItem: Rectangle { color: "#0078d4"; radius: 4; width: parent.width * parent.value }
                    }
                }
            }
        }

        // ── Stats Cards ───────────────────────────────────────────────────
        RowLayout {
            spacing: 20
            
            Rectangle {
                Layout.fillWidth: true; height: 120; radius: 12; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Space Reclaimed"; color: "#888"; font.pixelSize: 12 }
                    Label { text: gamePanel.totalSpaceReclaimedGb + " GB"; color: "#4caf50"; font.pixelSize: 32; font.bold: true }
                    Label { text: "Top 5% of users"; color: "#4caf50"; font.pixelSize: 10; opacity: 0.7 }
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 120; radius: 12; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Files Organized"; color: "#888"; font.pixelSize: 12 }
                    Label { text: gamePanel.filesOrganizedCount.toLocaleString(); color: "#ffaa00"; font.pixelSize: 32; font.bold: true }
                    Label { text: "Digital hoarder status: Cured"; color: "#ffaa00"; font.pixelSize: 10; opacity: 0.7 }
                }
            }

            Rectangle {
                Layout.fillWidth: true; height: 120; radius: 12; color: "#1a1a1a"; border.color: "#333"
                ColumnLayout { anchors.centerIn: parent; spacing: 4
                    Label { text: "Current Multiplier"; color: "#888"; font.pixelSize: 12 }
                    Label { text: "2.5x"; color: "#0078d4"; font.pixelSize: 32; font.bold: true }
                    Label { text: "Combo: 3 days streak"; color: "#0078d4"; font.pixelSize: 10; opacity: 0.7 }
                }
            }
        }

        // ── Achievements ──────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Unlocked Achievements"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
            padding: 15

            Flow {
                anchors.fill: parent; spacing: 15
                
                component Achievement: Rectangle {
                    width: 100; height: 100; radius: 50; color: unlocked ? "#1a3a5a" : "#111"
                    border.color: unlocked ? "#0078d4" : "#222"; border.width: 2
                    property bool unlocked: true
                    property string icon: "🏆"
                    property string title: ""
                    Column { anchors.centerIn: parent; spacing: 4
                        Label { text: parent.parent.icon; font.pixelSize: 32; anchors.horizontalCenter: parent.horizontalCenter }
                        Label { text: parent.parent.title; color: parent.parent.unlocked ? "white" : "#444"; font.pixelSize: 9; horizontalAlignment: Text.AlignHCenter; width: 80; wrapMode: Text.WordWrap }
                    }
                    ToolTip.visible: achHover.hovered; ToolTip.text: title + (unlocked ? " (Unlocked)" : " (Locked)")
                    HoverHandler { id: achHover }
                }

                Achievement { title: "Deduplicator"; icon: "👯" }
                Achievement { title: "Converter King"; icon: "🔄" }
                Achievement { title: "Privacy Shield"; icon: "🛡️" }
                Achievement { title: "Clean Sweep"; icon: "🧹" }
                Achievement { title: "Vault Master"; icon: "🔒" }
                Achievement { title: "Reflink Ninja"; icon: "⚡" }
                Achievement { title: "Metadata Medic", unlocked: false, icon: "🏷️" }
                Achievement { title: "Cloud Climber", unlocked: false, icon: "☁️" }
            }
        }

        // ── Daily Quest ───────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 80; radius: 10; color: "#0078d422"; border.color: "#0078d4"
            RowLayout {
                anchors.fill: parent; anchors.margins: 20; spacing: 15
                Label { text: "🎯 Daily Quest:"; color: "white"; font.bold: true }
                Label { text: "Organize 50 photos from /Downloads"; color: "#ccc"; Layout.fillWidth: true }
                Label { text: "Reward: +500 XP"; color: "#0078d4"; font.bold: true }
                Button { text: "Start"; background: Rectangle { color: "#0078d4"; radius: 4 }; contentItem: Label { text: parent.text; color: "white" } }
            }
        }
    }
}
