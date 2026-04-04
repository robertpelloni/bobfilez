import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniAudioPanel.qml — Professional DAW (Digital Audio Workstation) & AI Extractor.
/// Edit audio, apply VSTs, run Stem Separation, and identify tracks inside the shell.

Rectangle {
    id: audioPanel
    color: "#050505"; radius: 8

    property bool isPlaying: false
    property int currentBpm: 120
    property string activeProject: "New Project (48kHz)"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ── Top Control Bar ────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🎧 OmniAudio DAW"; font.pixelSize: 18; font.bold: true; color: "white" }
            Label { text: "AI Stem Extraction & Mixing"; color: "#888"; font.pixelSize: 12 }
            Item { Layout.fillWidth: true }
            
            // Transport Controls
            Rectangle {
                width: 250; height: 36; radius: 18; color: "#161616"; border.color: "#333"
                RowLayout {
                    anchors.centerIn: parent; spacing: 15
                    Button { text: "⏮"; flat: true; contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 16 } }
                    Button {
                        text: audioPanel.isPlaying ? "⏸" : "▶"
                        flat: true; onClicked: audioPanel.isPlaying = !audioPanel.isPlaying
                        contentItem: Label { text: parent.text; color: "#4caf50"; font.pixelSize: 20; font.bold: true }
                    }
                    Button { text: "⏺"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 16 } }
                    
                    Rectangle { width: 1; height: 20; color: "#444" }
                    
                    Label { text: "BPM"; color: "#888"; font.pixelSize: 10 }
                    TextInput { text: audioPanel.currentBpm.toString(); color: "#0078d4"; font.pixelSize: 14; font.bold: true; font.family: "Consolas" }
                }
            }

            Button {
                text: "🎛️ Export Mixdown"
                background: Rectangle { color: "#0078d4"; radius: 4 }
                contentItem: Label { text: parent.text; color: "white"; font.bold: true; font.pixelSize: 11 }
            }
        }

        // ── Workspace ──────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            // 1. AI Actions & Tools (Left)
            Rectangle {
                Layout.preferredWidth: 260; Layout.fillHeight: true
                color: "#161616"; border.color: "#333"; radius: 6
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 15
                    
                    Label { text: "AI AUDIO TOOLS"; color: "#aaa"; font.bold: true; font.pixelSize: 10 }
                    
                    // Shazam / Identify
                    GroupBox {
                        label: Label { text: "Identify Track"; color: "#888"; font.pixelSize: 10; font.bold: true }
                        Layout.fillWidth: true
                        background: Rectangle { color: "#111"; radius: 6; border.color: "#222" }
                        ColumnLayout {
                            anchors.fill: parent; spacing: 8
                            Button { text: "🔍 Shazam Acoustic FP"; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 }; contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter } }
                            Label { text: "Result: Around the World"; color: "#4caf50"; font.pixelSize: 11; font.bold: true }
                            Label { text: "Artist: Daft Punk"; color: "#ccc"; font.pixelSize: 10 }
                            Label { text: "Match: 98%"; color: "#888"; font.pixelSize: 9 }
                        }
                    }

                    // Stem Separation
                    GroupBox {
                        label: Label { text: "Demucs Stem Separation"; color: "#888"; font.pixelSize: 10; font.bold: true }
                        Layout.fillWidth: true
                        background: Rectangle { color: "#111"; radius: 6; border.color: "#222" }
                        ColumnLayout {
                            anchors.fill: parent; spacing: 8
                            Label { text: "Extract instruments from mixed track using PyTorch models."; color: "#666"; font.pixelSize: 9; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                            Button { text: "✂️ Split 4-Stems (Vocals/Drum/Bass)"; Layout.fillWidth: true; background: Rectangle { color: "#1a4a1a"; radius: 4; border.color: "#4caf50" }; contentItem: Label { text: parent.text; color: "#4caf50"; font.pixelSize: 11; font.bold: true; horizontalAlignment: Text.AlignHCenter } }
                        }
                    }

                    Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                    // VSTs / Plugins
                    Label { text: "DSP PLUGINS"; color: "#888"; font.bold: true; font.pixelSize: 10 }
                    ListView {
                        Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4
                        model: ["EQ Eight", "Compressor", "Reverb Pro", "Limiter", "Stereo Widener", "Auto-Tune"]
                        delegate: Rectangle {
                            width: parent.width; height: 30; color: "#1a1a1a"; radius: 4
                            RowLayout { anchors.fill: parent; anchors.margins: 8; Label { text: "🔌"; font.pixelSize: 12 }; Label { text: modelData; color: "#ccc"; font.pixelSize: 11; Layout.fillWidth: true } }
                        }
                    }
                }
            }

            // 2. Arrangement Timeline (Center)
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: "#111"; radius: 6; border.color: "#222"
                
                ColumnLayout {
                    anchors.fill: parent; spacing: 0
                    
                    // Timecode Ruler
                    Rectangle {
                        Layout.fillWidth: true; height: 24; color: "#1a1a1a"
                        Canvas {
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d"); ctx.strokeStyle = "#555"; ctx.lineWidth = 1;
                                for (let i = 0; i < width; i += 60) {
                                    ctx.beginPath(); ctx.moveTo(i, 12); ctx.lineTo(i, 24); ctx.stroke();
                                    ctx.fillStyle = "#888"; ctx.font = "9px Consolas";
                                    ctx.fillText("Bar " + (i/60 + 1), i+2, 10);
                                }
                            }
                        }
                    }

                    // Tracks
                    ListView {
                        Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 2
                        model: [
                            {name: "Vocals", color: "#8b5cf6", clips: [{x: 60, w: 300}, {x: 400, w: 150}]},
                            {name: "Drums", color: "#ffaa00", clips: [{x: 0, w: 600}]},
                            {name: "Bass", color: "#4caf50", clips: [{x: 60, w: 540}]},
                            {name: "Synths", color: "#0078d4", clips: [{x: 120, w: 200}, {x: 350, w: 250}]}
                        ]
                        delegate: RowLayout {
                            width: parent.width; height: 80; spacing: 0
                            
                            // Track Header
                            Rectangle {
                                width: 120; height: 80; color: "#161616"; border.color: "#2a2a2a"
                                ColumnLayout {
                                    anchors.fill: parent; anchors.margins: 6; spacing: 2
                                    Label { text: modelData.name; color: "white"; font.bold: true; font.pixelSize: 11; Layout.fillWidth: true }
                                    RowLayout {
                                        Rectangle { width: 18; height: 18; radius: 3; color: "#222"; Label { anchors.centerIn: parent; text: "M"; color: "#666"; font.pixelSize: 9 } }
                                        Rectangle { width: 18; height: 18; radius: 3; color: "#222"; Label { anchors.centerIn: parent; text: "S"; color: "#666"; font.pixelSize: 9 } }
                                        Rectangle { width: 18; height: 18; radius: 3; color: "#442222"; Label { anchors.centerIn: parent; text: "R"; color: "#ff4444"; font.pixelSize: 9 } }
                                    }
                                    Slider { Layout.fillWidth: true; from: -24; to: 6; value: 0 } // Volume
                                }
                            }

                            // Track Lane & Waveforms
                            Rectangle {
                                Layout.fillWidth: true; height: 80; color: "#0d0d0d"; border.color: "#222"
                                
                                Repeater {
                                    model: modelData.clips
                                    Rectangle {
                                        x: modelData.x; y: 4; width: modelData.w; height: 72; radius: 4
                                        color: parent.parent.modelData.color; opacity: 0.7; border.color: Qt.lighter(color)
                                        
                                        // Mock Waveform Drawing
                                        Canvas {
                                            anchors.fill: parent; anchors.margins: 2
                                            onPaint: {
                                                var ctx = getContext("2d"); ctx.strokeStyle = "#ffffff"; ctx.globalAlpha = 0.5; ctx.lineWidth = 1;
                                                for(let i=0; i<width; i+=3){ ctx.beginPath(); let h = Math.random()*(height-10); ctx.moveTo(i, height/2 - h/2); ctx.lineTo(i, height/2 + h/2); ctx.stroke(); }
                                            }
                                        }
                                        Label { anchors.top: parent.top; anchors.left: parent.left; anchors.leftMargin: 5; text: parent.parent.parent.modelData.name + " _take_1.wav"; color: "white"; font.pixelSize: 9; style: Text.Outline; styleColor: "black" }
                                    }
                                }
                            }
                        }
                    }
                }

                // Playhead Cursor
                Rectangle { x: 180; y: 0; width: 2; height: parent.height; color: "white"
                    Rectangle { anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top; width: 12; height: 12; color: "white"; radius: 6 }
                }
            }

            // 3. Mixer / Level Meters (Right)
            Rectangle {
                Layout.preferredWidth: 250; Layout.fillHeight: true
                color: "#161616"; border.color: "#333"; radius: 6
                
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 10
                    Label { text: "MASTER BUS"; color: "#aaa"; font.bold: true; font.pixelSize: 10 }
                    
                    RowLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 20; Layout.alignment: Qt.AlignHCenter
                        
                        // Faders and Meters
                        Repeater {
                            model: [{name: "Vocals", level: 0.8}, {name: "Drums", level: 0.9}, {name: "Master", level: 0.75}]
                            ColumnLayout {
                                spacing: 5
                                // VU Meter
                                Rectangle {
                                    width: 14; Layout.fillHeight: true; color: "#111"; radius: 2; border.color: "#222"
                                    Rectangle {
                                        width: 12; height: parent.height * modelData.level; anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; radius: 1
                                        gradient: Gradient { GradientStop { position: 0.0; color: "#ff4444" } GradientStop { position: 0.3; color: "#ffaa00" } GradientStop { position: 1.0; color: "#4caf50" } }
                                    }
                                }
                                // Fader Slider (Vertical)
                                Slider {
                                    orientation: Qt.Vertical; from: -60; to: 6; value: 0; Layout.fillHeight: true
                                }
                                Label { text: modelData.name; color: index === 2 ? "#0078d4" : "#888"; font.pixelSize: 9; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                            }
                        }
                    }
                }
            }
        }
    }
}
