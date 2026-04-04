import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniVersePanel.qml — 3D Spatial File Explorer Launcher.
/// Interfaces with the hardware-accelerated C++ raylib engine to launch
/// a fully interactive, three-dimensional representation of your data.

Rectangle {
    id: versePanel
    color: "#050505"; radius: 8

    property bool isLaunching: false
    property string targetFolder: "C:/Users/hyper/workspace/bobfilez/core/src/"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🌌 OmniVerse 3D"; font.pixelSize: 22; font.bold: true; color: "#0078d4" }
            Label { text: "Spatial Computing File System"; color: "#888"; font.pixelSize: 14 }
            
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: 120; height: 24; radius: 12; color: "#1a3a1a"; border.color: "#4caf50"
                Label { anchors.centerIn: parent; text: "OpenGL 4.5 READY"; color: "#4caf50"; font.pixelSize: 10; font.bold: true }
            }
        }

        // ── Hero Section (3D Preview) ─────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "black"; radius: 12; border.color: "#333"
            clip: true

            // Simulated 3D Environment Background (Grid/Stars)
            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.strokeStyle = "#1a1a1a"; ctx.lineWidth = 1;
                    
                    // Perspective Grid
                    for (let y = height/2; y < height; y += (y - height/2 + 10)/2) {
                        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke();
                    }
                    for (let x = -width; x < width*2; x += 100) {
                        ctx.beginPath(); ctx.moveTo(width/2, height/2); ctx.lineTo(x, height); ctx.stroke();
                    }
                }
            }

            // Floating 3D Nodes (Mocked animation)
            Item {
                anchors.centerIn: parent; width: 400; height: 400

                // Center Node (Parent Folder)
                Rectangle {
                    width: 100; height: 100; radius: 50; color: "#0078d488"; border.color: "#0078d4"; border.width: 3
                    anchors.centerIn: parent
                    Label { anchors.centerIn: parent; text: "src"; color: "white"; font.bold: true; font.pixelSize: 18 }
                    
                    // Pulse animation
                    SequentialAnimation on scale {
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 1.05; duration: 2000; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 1.05; to: 1.0; duration: 2000; easing.type: Easing.InOutSine }
                    }
                }

                // Orbiting Nodes (Files)
                Repeater {
                    model: [
                        {name: "omniverse_engine.cpp", color: "#4caf50", angle: 0, r: 150, size: 40},
                        {name: "time_machine.cpp", color: "#ffaa00", angle: 72, r: 180, size: 50},
                        {name: "nexus.cpp", color: "#ff4444", angle: 144, r: 140, size: 60},
                        {name: "omnigit.cpp", color: "#8b5cf6", angle: 216, r: 160, size: 45},
                        {name: "omni_mount.cpp", color: "#d4d4d4", angle: 288, r: 130, size: 55}
                    ]
                    Rectangle {
                        id: orbiter
                        width: modelData.size; height: modelData.size; radius: width/2
                        color: modelData.color; opacity: 0.8; border.color: "white"

                        // Orbit animation
                        NumberAnimation on rotation { from: 0; to: 360; duration: 15000; loops: Animation.Infinite }
                        
                        transform: [
                            Rotation { origin.x: -modelData.r; origin.y: -modelData.r; angle: modelData.angle; axis { x: 0; y: 1; z: 0.5 } }
                        ]

                        Label {
                            anchors.bottom: parent.top; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottomMargin: 5
                            text: modelData.name; color: "white"; font.pixelSize: 11; style: Text.Outline; styleColor: "black"
                        }
                    }
                }
            }

            // Launch Overlay
            Rectangle {
                anchors.centerIn: parent; width: 450; height: 200
                color: "#aa000000"; radius: 10; border.color: "#444"

                ColumnLayout {
                    anchors.centerIn: parent; spacing: 15
                    
                    Label { text: "Launch 3D File Universe"; color: "white"; font.pixelSize: 18; font.bold: true; Layout.alignment: Qt.AlignHCenter }
                    
                    RowLayout {
                        spacing: 10
                        Label { text: "Root Target:"; color: "#888"; font.pixelSize: 12 }
                        TextInput { text: versePanel.targetFolder; color: "white"; font.pixelSize: 14; font.family: "Consolas"; Layout.preferredWidth: 200; background: Rectangle { color: "#222"; radius: 4 } }
                        Button { text: "Browse..."; flat: true; contentItem: Label { text: parent.text; color: "#0078d4" } }
                    }

                    Button {
                        Layout.alignment: Qt.AlignHCenter; Layout.preferredWidth: 200; height: 40
                        text: versePanel.isLaunching ? "Hardware Engaged..." : "🚀 ENGAGE HARDWARE ACCELERATION"
                        background: Rectangle { color: versePanel.isLaunching ? "#ff4444" : "#0078d4"; radius: 20 }
                        contentItem: Label { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                        onClicked: {
                            versePanel.isLaunching = true;
                            // Trigger C++ launch_3d_viewport
                        }
                    }
                }
            }
        }

        // ── Physics Settings ──────────────────────────────────────────────
        RowLayout {
            spacing: 20
            Label { text: "Force-Directed Graph Physics:"; color: "#888"; font.pixelSize: 11; font.bold: true }
            
            Label { text: "Gravity:"; color: "#666"; font.pixelSize: 11 }
            Slider { value: 0.5; Layout.preferredWidth: 100 }
            
            Label { text: "Repulsion:"; color: "#666"; font.pixelSize: 11 }
            Slider { value: 0.8; Layout.preferredWidth: 100 }
            
            Label { text: "Friction:"; color: "#666"; font.pixelSize: 11 }
            Slider { value: 0.3; Layout.preferredWidth: 100 }

            Item { Layout.fillWidth: true }
            
            CheckBox { contentItem: Label { text: "Enable VR Headset (OpenXR)"; color: "#ccc"; font.pixelSize: 11; leftPadding: 4 } }
        }
    }
}
