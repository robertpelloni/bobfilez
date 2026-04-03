import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// VisualDedupPanel.qml — "Tinder-style" duplicate confirm/reject UI.
/// Designed for quickly cleaning up near-duplicate photos or burst shots.
/// Users swipe right to KEEP/GROUP, swipe left to DELETE/REMOVE.

Rectangle {
    id: visualDedup
    color: "#0f0f0f"; radius: 8

    property var duplicateGroups: [] // [{id, files: [{path, score, ...}]}]
    property int currentGroupIndex: 0
    property int currentFileIndex: 0
    property bool isRunning: false

    signal groupConfirmed(int groupId)
    signal fileDeleted(string path)
    signal skipGroup()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            Label { text: "🔥 Visual Discovery"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Near-Duplicate Image Cleaner"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            Label { text: "Progress: " + (currentGroupIndex + 1) + " / " + duplicateGroups.length; color: "#0078d4"; font.bold: true }
        }

        // ── Main Interaction Area ─────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // The "Card Stack"
            Item {
                id: cardStack
                anchors.centerIn: parent
                width: 500; height: 600

                // Background cards (stacked effect)
                Repeater {
                    model: 3
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - (index * 20); height: parent.height - (index * 20)
                        z: -index
                        color: "#1a1a1a"
                        radius: 15; border.color: "#333"
                        opacity: 1.0 - (index * 0.3)
                        rotation: index * 2
                        visible: (visualDedup.currentGroupIndex + index) < visualDedup.duplicateGroups.length
                    }
                }

                // Top Active Card
                Rectangle {
                    id: activeCard
                    anchors.fill: parent
                    color: "#1e1e1e"; radius: 15; border.color: "#444"; border.width: 1
                    clip: true

                    ColumnLayout {
                        anchors.fill: parent; spacing: 0

                        // Image Preview
                        Rectangle {
                            Layout.fillWidth: true; Layout.fillHeight: true; color: "black"; radius: 15
                            Image {
                                id: mainImg
                                anchors.fill: parent; anchors.margins: 10
                                source: "image:/thumb/placeholder" // Wired to current group's leader
                                fillMode: Image.PreserveAspectFit
                            }
                            
                            // Badge for similarity score
                            Rectangle {
                                anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 20
                                width: 60; height: 30; radius: 15; color: "#0078d4"
                                Label { anchors.centerIn: parent; text: "98%"; color: "white"; font.bold: true }
                            }
                        }

                        // Info Footer
                        Rectangle {
                            Layout.fillWidth: true; height: 100; color: "transparent"
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 4
                                Label { text: "IMG_2024_07_12.JPG"; color: "white"; font.pixelSize: 16; font.bold: true }
                                Label { text: "4032 x 3024 • 4.2 MB • JPEG"; color: "#888"; font.pixelSize: 12 }
                                Label { text: "Found in: /Users/robert/Pictures/Summer"; color: "#666"; font.pixelSize: 11 }
                            }
                        }
                    }

                    // Swipe Gestures
                    MouseArea {
                        anchors.fill: parent
                        property real startX
                        onPressed: startX = mouse.x
                        onPositionChanged: {
                            let delta = mouse.x - startX
                            activeCard.rotation = delta / 10
                            activeCard.x = delta
                        }
                        onReleased: {
                            if (activeCard.x > 150) {
                                // Swipe Right -> KEEP
                                swipeRightAnim.start()
                            } else if (activeCard.x < -150) {
                                // Swipe Left -> DELETE
                                swipeLeftAnim.start()
                            } else {
                                // Snap back
                                snapBackAnim.start()
                            }
                        }
                    }

                    NumberAnimation { id: swipeRightAnim; target: activeCard; property: "x"; to: 1000; duration: 250; onFinished: { visualDedup.next() } }
                    NumberAnimation { id: swipeLeftAnim; target: activeCard; property: "x"; to: -1000; duration: 250; onFinished: { visualDedup.next() } }
                    ParallelAnimation {
                        id: snapBackAnim
                        NumberAnimation { target: activeCard; property: "x"; to: 0; duration: 200; easing.type: Easing.OutBack }
                        NumberAnimation { target: activeCard; property: "rotation"; to: 0; duration: 200; easing.type: Easing.OutBack }
                    }
                }
            }
        }

        // ── Controls ──────────────────────────────────────────────────────
        RowLayout {
            Layout.alignment: Qt.AlignHCenter; spacing: 40
            
            // Reject Button
            Button {
                id: btnReject
                background: Rectangle { width: 70; height: 70; radius: 35; color: "#2a1010"; border.color: "#ff4444"
                    Label { anchors.centerIn: parent; text: "✖"; color: "#ff4444"; font.pixelSize: 30 } }
                onClicked: swipeLeftAnim.start()
            }

            // Skip Button
            Button {
                text: "Skip Group"; flat: true
                contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 14 }
                onClicked: visualDedup.currentGroupIndex++
            }

            // Keep Button
            Button {
                id: btnKeep
                background: Rectangle { width: 70; height: 70; radius: 35; color: "#102a10"; border.color: "#4caf50"
                    Label { anchors.centerIn: parent; text: "✔"; color: "#4caf50"; font.pixelSize: 30 } }
                onClicked: swipeRightAnim.start()
            }
        }

        // ── Comparison List ───────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 120; color: "#111"; radius: 10; border.color: "#222"
            RowLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 15
                Label { text: "Matches in Group:"; color: "#aaa"; font.bold: true; Layout.preferredWidth: 120 }
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    orientation: ListView.Horizontal; spacing: 10; model: 4
                    delegate: Rectangle {
                        width: 100; height: 100; radius: 8; color: "#1a1a1a"; border.color: index === 0 ? "#0078d4" : "#333"
                        Image { anchors.fill: parent; anchors.margins: 4; source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectFit }
                        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 20; color: "#cc000000"; radius: 4
                            Label { anchors.centerIn: parent; text: "99%"; color: "white"; font.pixelSize: 10 } }
                    }
                }
            }
        }
    }

    function next() {
        activeCard.x = 0
        activeCard.rotation = 0
        currentGroupIndex++
    }
}
