import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniVisionPanel.qml — Professional NLE (Non-Linear Editor) and Color Grading Suite.
/// Edit videos, arrange clips on an OpenTimelineIO track, apply OpenColorIO LUTs,
/// and render using FFmpeg directly inside the file manager.

Rectangle {
    id: visionPanel
    color: "#050505"; radius: 8

    property bool isPlaying: false
    property real currentFrameTime: 12.5
    property real timelineDuration: 60.0 // seconds

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ── Top Bar ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🎬 OmniVision NLE"; font.pixelSize: 18; font.bold: true; color: "white" }
            Label { text: "OpenTimelineIO + OpenColorIO"; color: "#888"; font.pixelSize: 12 }
            Item { Layout.fillWidth: true }
            
            Button {
                text: "📤 Export (FFmpeg / OTIO)"
                background: Rectangle { color: "#0078d4"; radius: 4 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true; font.pixelSize: 11 }
            }
        }

        // ── Workspace ──────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            // 1. Media Pool (Left)
            Rectangle {
                Layout.preferredWidth: 240; Layout.fillHeight: true
                color: "#161616"; border.color: "#333"; radius: 6
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 10
                    Label { text: "MEDIA POOL"; color: "#aaa"; font.bold: true; font.pixelSize: 10 }
                    
                    ListView {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 5; clip: true
                        model: [
                            {name: "drone_shot_01.mp4", duration: "00:15", type: "V"},
                            {name: "interview_cam_A.mov", duration: "02:30", type: "V"},
                            {name: "b_roll_city.mkv", duration: "00:45", type: "V"},
                            {name: "ambient_music.wav", duration: "03:10", type: "A"}
                        ]
                        delegate: Rectangle {
                            width: parent.width; height: 40; color: itemHover.hovered ? "#252525" : "#1a1a1a"; radius: 4
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 8; spacing: 10
                                Label { text: modelData.type === "V" ? "🎬" : "🎵"; font.pixelSize: 14 }
                                ColumnLayout {
                                    spacing: 2; Layout.fillWidth: true
                                    Label { text: modelData.name; color: "white"; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
                                    Label { text: modelData.duration; color: "#888"; font.pixelSize: 9 }
                                }
                            }
                            HoverHandler { id: itemHover }
                            // Drag support would go here to drop onto timeline
                        }
                    }
                }
            }

            // 2. Video Monitor (Center)
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: "black"; radius: 6; border.color: "#222"
                
                // MPV Video Player Placeholder
                Image {
                    anchors.fill: parent; anchors.margins: 20
                    source: "image:/thumb/placeholder"; fillMode: Image.PreserveAspectFit
                }
                
                // Playback Controls
                Rectangle {
                    anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: 15; width: 250; height: 40; radius: 20; color: "#aa000000"
                    border.color: "#444"
                    
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 15
                        Button { text: "⏮"; flat: true; contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 16 } }
                        Button {
                            text: visionPanel.isPlaying ? "⏸" : "▶"
                            flat: true; onClicked: visionPanel.isPlaying = !visionPanel.isPlaying
                            contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 20 }
                        }
                        Button { text: "⏭"; flat: true; contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 16 } }
                        Item { Layout.fillWidth: true }
                        Label { text: "00:00:12:15"; color: "#0078d4"; font.pixelSize: 12; font.family: "Consolas" }
                    }
                }
            }

            // 3. Inspector & Color Grading (Right)
            Rectangle {
                Layout.preferredWidth: 300; Layout.fillHeight: true
                color: "#161616"; border.color: "#333"; radius: 6
                
                ScrollView {
                    anchors.fill: parent; clip: true
                    ColumnLayout {
                        width: parent.width - 20; anchors.margins: 10; spacing: 15
                        
                        Label { text: "OPENCOLORIO GRADING"; color: "#aaa"; font.bold: true; font.pixelSize: 10 }
                        
                        // LUT Selector
                        ColumnLayout {
                            spacing: 4; Layout.fillWidth: true
                            Label { text: "Input Color Space"; color: "#888"; font.pixelSize: 10 }
                            ComboBox { model: ["S-Log3", "V-Log", "C-Log", "Rec.709"]; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 } contentItem: Label { text: parent.currentText; color: "white"; font.pixelSize: 11; leftPadding: 8 } }
                            
                            Label { text: "Apply 3D LUT"; color: "#888"; font.pixelSize: 10; Layout.topMargin: 10 }
                            ComboBox { model: ["None", "Cinematic Teal & Orange", "Kodak 2383 Film", "Bleach Bypass"]; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 } contentItem: Label { text: parent.currentText; color: "white"; font.pixelSize: 11; leftPadding: 8 } }
                        }
                        
                        Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                        // Basic Wheels
                        Label { text: "COLOR WHEELS"; color: "#888"; font.bold: true; font.pixelSize: 10 }
                        RowLayout {
                            spacing: 15; Layout.alignment: Qt.AlignHCenter
                            Column {
                                Rectangle { width: 60; height: 60; radius: 30; border.color: "#444"; color: "transparent"
                                    Rectangle { width: 8; height: 8; radius: 4; color: "white"; anchors.centerIn: parent } }
                                Label { text: "Lift"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                            }
                            Column {
                                Rectangle { width: 60; height: 60; radius: 30; border.color: "#444"; color: "transparent"
                                    Rectangle { width: 8; height: 8; radius: 4; color: "white"; anchors.centerIn: parent } }
                                Label { text: "Gamma"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                            }
                            Column {
                                Rectangle { width: 60; height: 60; radius: 30; border.color: "#444"; color: "transparent"
                                    Rectangle { width: 8; height: 8; radius: 4; color: "white"; anchors.centerIn: parent } }
                                Label { text: "Gain"; color: "#888"; font.pixelSize: 9; anchors.horizontalCenter: parent.horizontalCenter }
                            }
                        }

                        Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                        // MediaInfo (Submodule integration)
                        Label { text: "MEDIA INFO (Active Clip)"; color: "#aaa"; font.bold: true; font.pixelSize: 10 }
                        ColumnLayout {
                            spacing: 4; Layout.fillWidth: true
                            RowLayout { Label { text: "Codec:"; color: "#666"; font.pixelSize: 10; width: 80 }; Label { text: "HEVC/H.265 10-bit"; color: "#ccc"; font.pixelSize: 10 } }
                            RowLayout { Label { text: "Resolution:"; color: "#666"; font.pixelSize: 10; width: 80 }; Label { text: "3840x2160 (4K UHD)"; color: "#ccc"; font.pixelSize: 10 } }
                            RowLayout { Label { text: "Framerate:"; color: "#666"; font.pixelSize: 10; width: 80 }; Label { text: "59.94 fps"; color: "#ccc"; font.pixelSize: 10 } }
                            RowLayout { Label { text: "Bitrate:"; color: "#666"; font.pixelSize: 10; width: 80 }; Label { text: "150 Mbps (All-I)"; color: "#ccc"; font.pixelSize: 10 } }
                        }
                    }
                }
            }
        }

        // ── Timeline (OpenTimelineIO) ──────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 200
            color: "#161616"; border.color: "#333"; radius: 6

            ColumnLayout {
                anchors.fill: parent; spacing: 0
                
                // Timeline Header (Timecode ruler)
                Rectangle {
                    Layout.fillWidth: true; height: 24; color: "#222"
                    // Ruler markers would be drawn here
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d"); ctx.strokeStyle = "#555"; ctx.lineWidth = 1;
                            for (let i = 0; i < width; i += 50) {
                                ctx.beginPath(); ctx.moveTo(i, 12); ctx.lineTo(i, 24); ctx.stroke();
                                ctx.fillStyle = "#888"; ctx.font = "9px Consolas";
                                ctx.fillText("00:00:" + (i/10).toFixed(0).padStart(2,"0"), i+2, 10);
                            }
                        }
                    }
                }

                // Tracks
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 2
                    model: [
                        {track: "V2", color: "#4285f4", type: "Video"},
                        {track: "V1", color: "#4285f4", type: "Video"},
                        {track: "A1", color: "#34a853", type: "Audio"},
                        {track: "A2", color: "#34a853", type: "Audio"}
                    ]
                    delegate: RowLayout {
                        width: parent.width; height: 40; spacing: 0
                        
                        // Track Header
                        Rectangle {
                            width: 60; height: 40; color: "#111"; border.color: "#2a2a2a"
                            Label { anchors.centerIn: parent; text: modelData.track; color: "#888"; font.bold: true; font.pixelSize: 11 }
                        }

                        // Track Lane
                        Rectangle {
                            Layout.fillWidth: true; height: 40; color: "#0d0d0d"; border.color: "#222"
                            
                            // Mock Clips
                            Rectangle {
                                visible: index === 1
                                x: 50; y: 4; width: 250; height: 32; radius: 4
                                color: modelData.color; opacity: 0.8; border.color: Qt.lighter(color)
                                Label { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 8; text: "drone_shot_01.mp4"; color: "white"; font.pixelSize: 10 }
                            }
                            
                            Rectangle {
                                visible: index === 1
                                x: 305; y: 4; width: 180; height: 32; radius: 4
                                color: modelData.color; opacity: 0.8; border.color: Qt.lighter(color)
                                Label { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 8; text: "interview_cam_A.mov"; color: "white"; font.pixelSize: 10 }
                            }
                            
                            Rectangle {
                                visible: index === 2
                                x: 50; y: 4; width: 435; height: 32; radius: 4
                                color: modelData.color; opacity: 0.8; border.color: Qt.lighter(color)
                                Label { anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 8; text: "ambient_music.wav"; color: "white"; font.pixelSize: 10 }
                                // Audio waveform mock
                                Canvas { anchors.fill: parent; anchors.margins: 2; onPaint: { var ctx=getContext("2d"); ctx.strokeStyle="#ffffff44"; for(let i=0;i<width;i+=4){ ctx.beginPath(); ctx.moveTo(i, height/2 - Math.random()*12); ctx.lineTo(i, height/2 + Math.random()*12); ctx.stroke(); } } }
                            }
                        }
                    }
                }
            }

            // Playhead Cursor
            Rectangle {
                x: 180; y: 0; width: 2; height: parent.height; color: "#ff4444"
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top
                    width: 12; height: 12; color: "#ff4444"; radius: 2
                }
            }
        }
    }
}
