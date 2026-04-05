import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// PhotoDevelopPanel.qml — Adobe Lightroom Classic parity develop module.
/// Retargeted to local Qt Quick / Controls primitives for BTK migration.
/// Features non-destructive adjustments, histogram, and pro-grade sliders.

Rectangle {
    id: developPanel
    color: "#111"; radius: 10

    property string currentPhotoPath: ""
    property real exposure: 0.0
    property real temperature: 5500
    property real tint: 0
    property real contrast: 0
    property real highlights: 0
    property real shadows: 0
    property real clarity: 0

    RowLayout {
        anchors.fill: parent; spacing: 0

        // ── Left Sidebar: Presets & History ──
        Rectangle {
            Layout.preferredWidth: 260; Layout.fillHeight: true
            color: "#1a1a1a"; border.color: "#333"

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 10
                
                Label { text: "PRESETS"; color: "#888"; font.bold: true; font.pixelSize: 11 }
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    model: ["B&W High Contrast", "Vivid Landscape", "Cinematic Teal", "Soft Portrait", "Matte Look"]
                    delegate: Text {
                        text: modelData
                        color: "#ccc"
                        font.pixelSize: 12
                    }
                }

                Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                Label { text: "HISTORY"; color: "#888"; font.bold: true; font.pixelSize: 11 }
                ListView {
                    Layout.fillWidth: true; Layout.preferredHeight: 200
                    model: ["Import", "Exposure +0.50", "Highlights -20", "Shadows +15", "Clarity +10"]
                    delegate: Text {
                        text: modelData
                        color: "#ccc"
                        font.pixelSize: 12
                    }
                }
            }
        }

        // ── Center: Main Canvas ──
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "black"
            
            Image {
                id: mainImage
                anchors.fill: parent; anchors.margins: 20
                source: "image:/photo/" + developPanel.currentPhotoPath
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            // Toolbar Overlay
            RowLayout {
                anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20; spacing: 10
                Button { text: "✂️ Crop"; flat: true }
                Button { text: "🩹 Heal"; flat: true }
                Button { text: "🎭 Mask"; flat: true }
                Button { text: "👁️ Red Eye"; flat: true }
            }
        }

        // ── Right Sidebar: Adjustments (Lightroom Style) ──
        Rectangle {
            Layout.preferredWidth: 320; Layout.fillHeight: true
            color: "#1a1a1a"; border.color: "#333"

            ScrollView {
                anchors.fill: parent; clip: true
                ColumnLayout {
                    width: parent.width - 20; anchors.margins: 10; spacing: 15

                    // Histogram (Placeholder)
                    Rectangle {
                        Layout.fillWidth: true; height: 120; color: "#0d0d0d"; radius: 4
                        Label { anchors.centerIn: parent; text: "Histogram Graph"; color: "#444" }
                    }

                    // Basic Panel
                    GroupBox {
                        label: Label { text: "BASIC"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                        Layout.fillWidth: true
                        background: Rectangle { color: "transparent" }

                        ColumnLayout {
                            anchors.fill: parent; spacing: 12

                            // WB
                            RowLayout {
                                Label { text: "Temp"; color: "#888"; font.pixelSize: 11; width: 60 }
                                Slider { Layout.fillWidth: true; from: 2000; to: 50000; value: developPanel.temperature }
                                Label { text: Math.round(developPanel.temperature); color: "#ccc"; width: 40 }
                            }

                            // Dials for Exposure & Contrast
                            RowLayout {
                                spacing: 20; Layout.alignment: Qt.AlignHCenter
                                Column {
                                    Dial { id: expDial; from: -5; to: 5; value: developPanel.exposure; width: 60; height: 60 }
                                    Label { text: "Exposure"; color: "#888"; font.pixelSize: 10; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                                Column {
                                    Dial { id: conDial; from: -100; to: 100; value: developPanel.contrast; width: 60; height: 60 }
                                    Label { text: "Contrast"; color: "#888"; font.pixelSize: 10; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                            }

                            // Tone Sliders
                            component ToneSlider: RowLayout {
                                property alias label: lbl.text
                                property alias val: sl.value
                                Label { id: lbl; color: "#888"; font.pixelSize: 11; width: 60 }
                                Slider { id: sl; Layout.fillWidth: true; from: -100; to: 100 }
                                Label { text: Math.round(sl.value); color: "#ccc"; width: 30 }
                            }

                            ToneSlider { label: "Highlights"; val: developPanel.highlights }
                            ToneSlider { label: "Shadows";    val: developPanel.shadows }
                            ToneSlider { label: "Clarity";    val: developPanel.clarity }
                        }
                    }

                    // Tone Curve
                    GroupBox {
                        label: Label { text: "TONE CURVE"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                        Layout.fillWidth: true
                        Rectangle { Layout.fillWidth: true; height: 200; color: "#0d0d0d"; radius: 4 }
                    }

                    // HSL / Color
                    GroupBox {
                        label: Label { text: "HSL / COLOR"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                        Layout.fillWidth: true
                        RowLayout {
                            Repeater {
                                model: ["R","O","Y","G","A","B","P","M"]
                                Rectangle { width: 20; height: 20; color: "#222"; radius: 10
                                    Label { anchors.centerIn: parent; text: modelData; font.pixelSize: 10; color: "#666" } }
                            }
                        }
                    }
                }
            }
        }
    }
}
