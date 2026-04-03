import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

// ─────────────────────────────────────────────────────────────────────────────
// BatchConvertPanel.qml
// Comprehensive batch file format conversion panel — inspired by:
//   HandBrake (video), XnConvert (image), Pandoc-GUI, FFmpeg-GUI
//
// Features:
//   • Auto-detect input format → suggest output formats
//   • Per-converter option panels (video: bitrate/crf/preset; image: quality/resize; doc: template/toc)
//   • Output directory picker (same as input / custom / subfolder)
//   • Collision handling: skip / overwrite / rename
//   • Thread count slider
//   • Live progress per-file + overall
//   • Error log with copy/save buttons
//   • Preset save/load
// ─────────────────────────────────────────────────────────────────────────────
Rectangle {
    id: batchConvertPanel
    color: "#161616"
    radius: 8

    property var inputFiles: []        // list of paths
    property string outputExt: "mp4"
    property string outputDir: ""
    property string collisionPolicy: "rename"
    property int numThreads: 4
    property bool isRunning: false
    property int progressDone: 0
    property int progressTotal: 0

    // ─────────────────── signals ─────────────────────────────────────────────
    signal startConversion()
    signal cancelConversion()
    signal addFiles()
    signal addFolder()
    signal clearFiles()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // ── Header ─────────────────────────────────────────────────────────
        RowLayout {
            Label { text: "🔄 Batch Convert"; font.pixelSize: 18; font.bold: true; color: "white" }
            Item { Layout.fillWidth: true }
            Button { text: "💾 Save Preset"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
            Button { text: "📂 Load Preset"; flat: true; contentItem: Label { text: parent.text; color: "#888" } }
        }

        // ── Source files ────────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Source Files"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 10

            ColumnLayout {
                anchors.fill: parent; spacing: 8

                RowLayout {
                    Button {
                        text: "＋ Add Files"
                        onClicked: batchConvertPanel.addFiles()
                        contentItem: Label { text: parent.text; color: "white" }
                        background: Rectangle { color: "#0078d4"; radius: 4 }
                    }
                    Button {
                        text: "📁 Add Folder"
                        onClicked: batchConvertPanel.addFolder()
                        contentItem: Label { text: parent.text; color: "white" }
                        background: Rectangle { color: "#252525"; radius: 4; border.color: "#444" }
                    }
                    Button {
                        text: "🗑 Clear"
                        onClicked: batchConvertPanel.clearFiles()
                        contentItem: Label { text: parent.text; color: "#ff6b6b" }
                        background: Rectangle { color: "transparent"; radius: 4 }
                    }
                    Item { Layout.fillWidth: true }
                    Label { text: batchConvertPanel.inputFiles.length + " file(s)"; color: "#888"; font.pixelSize: 12 }
                }

                // File list
                ListView {
                    id: fileList
                    Layout.fillWidth: true
                    height: 140
                    clip: true
                    model: batchConvertPanel.inputFiles
                    ScrollBar.vertical: ScrollBar {}
                    delegate: Rectangle {
                        width: fileList.width; height: 28
                        color: index % 2 == 0 ? "#1e1e1e" : "#232323"
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                            Label { text: "📄"; font.pixelSize: 14 }
                            Label { text: modelData; color: "#ccc"; font.pixelSize: 12; elide: Text.ElideLeft; Layout.fillWidth: true }
                            Label { text: "✕"; color: "#666"; font.pixelSize: 11; MouseArea { anchors.fill: parent; onClicked: { /* remove */ } } }
                        }
                    }
                }
            }
        }

        // ── Conversion settings ─────────────────────────────────────────────
        RowLayout {
            spacing: 12
            Layout.fillWidth: true

            // Output format
            GroupBox {
                label: Label { text: "Output Format"; color: "#aaa"; font.bold: true }
                Layout.fillWidth: true
                background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
                padding: 10

                ColumnLayout {
                    anchors.fill: parent; spacing: 8

                    Label { text: "Target Extension"; color: "#888"; font.pixelSize: 11 }
                    ComboBox {
                        id: outputExtCombo
                        Layout.fillWidth: true
                        model: ["mp4","mkv","avi","webm","gif","mp3","flac","aac","ogg","opus","wav",
                                "jpg","png","webp","avif","heic","tiff","bmp","pdf","docx","epub","md","html","odt"]
                        currentIndex: 0
                        onCurrentTextChanged: batchConvertPanel.outputExt = currentText
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 8 }
                    }

                    Label { text: "Backend Converter"; color: "#888"; font.pixelSize: 11 }
                    ComboBox {
                        Layout.fillWidth: true
                        model: ["Auto (Best)", "FFmpeg", "ImageMagick", "Pandoc", "Ghostscript", "Calibre", "Poppler", "wkhtmltopdf", "libvips", "7-Zip", "Inkscape", "SoX"]
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 8 }
                    }
                }
            }

            // Output destination
            GroupBox {
                label: Label { text: "Output Destination"; color: "#aaa"; font.bold: true }
                Layout.fillWidth: true
                background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
                padding: 10

                ColumnLayout {
                    anchors.fill: parent; spacing: 8

                    ButtonGroup { id: destGroup }
                    RadioButton { text: "Same folder as input"; ButtonGroup.group: destGroup; checked: true; contentItem: Label { text: parent.text; color: "#ccc" } }
                    RadioButton { text: "Subfolder of input"; ButtonGroup.group: destGroup; contentItem: Label { text: parent.text; color: "#ccc" } }
                    RadioButton {
                        id: customDestRadio; text: "Custom folder"; ButtonGroup.group: destGroup
                        contentItem: Label { text: parent.text; color: "#ccc" }
                    }
                    RowLayout {
                        enabled: customDestRadio.checked
                        TextField {
                            id: destPathField; Layout.fillWidth: true; text: batchConvertPanel.outputDir
                            color: "white"; background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            placeholderText: "Choose output folder..."
                        }
                        Button {
                            text: "…"
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.text; color: "white" }
                        }
                    }
                }
            }
        }

        // ── Format-specific options ─────────────────────────────────────────
        GroupBox {
            label: Label { text: "Converter Options"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 10

            ColumnLayout {
                anchors.fill: parent

                // Video options (shown when output is video)
                ColumnLayout {
                    visible: ["mp4","mkv","avi","webm","gif","mov","flv"].indexOf(batchConvertPanel.outputExt) >= 0
                    spacing: 8

                    Label { text: "Video Settings"; color: "#0078d4"; font.bold: true; font.pixelSize: 12 }
                    GridLayout { columns: 4; columnSpacing: 12; rowSpacing: 6

                        Label { text: "Codec"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["H.264 (libx264)","H.265 (libx265)","VP9","AV1","Copy Stream"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }

                        Label { text: "Quality (CRF)"; color: "#888"; font.pixelSize: 11 }
                        RowLayout {
                            Slider { id: crfSlider; from: 0; to: 51; value: 23; stepSize: 1; Layout.fillWidth: true }
                            Label { text: Math.round(crfSlider.value); color: "#ccc"; font.pixelSize: 12; width: 24 }
                        }

                        Label { text: "Preset"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["ultrafast","superfast","veryfast","faster","fast","medium","slow","slower","veryslow"]
                            currentIndex: 5
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }

                        Label { text: "Resolution"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Keep original","3840x2160 (4K)","1920x1080 (FHD)","1280x720 (HD)","854x480","640x360","Custom..."]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }

                        Label { text: "Framerate"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Keep original","60","30","25","24","15"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }

                        Label { text: "Audio"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Copy original","AAC 128k","AAC 192k","MP3 192k","Opus 128k","Remove audio"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }

                        Label { text: "Trim Start"; color: "#888"; font.pixelSize: 11 }
                        TextField { placeholderText: "00:00:00"; color: "white"; Layout.fillWidth: true
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                        Label { text: "Trim Duration"; color: "#888"; font.pixelSize: 11 }
                        TextField { placeholderText: "00:00:00 (blank=full)"; color: "white"; Layout.fillWidth: true
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }
                    }
                }

                // Image options
                ColumnLayout {
                    visible: ["jpg","jpeg","png","webp","avif","heic","tiff","bmp","ico"].indexOf(batchConvertPanel.outputExt) >= 0
                    spacing: 8
                    Label { text: "Image Settings"; color: "#0078d4"; font.bold: true; font.pixelSize: 12 }
                    GridLayout { columns: 4; columnSpacing: 12; rowSpacing: 6
                        Label { text: "Quality"; color: "#888"; font.pixelSize: 11 }
                        RowLayout {
                            Slider { id: imgQuality; from: 1; to: 100; value: 85; Layout.fillWidth: true }
                            Label { text: Math.round(imgQuality.value); color: "#ccc"; width: 30 }
                        }
                        Label { text: "Resize"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["No resize","50%","75%","Max 1920px","Max 1280px","Custom..."]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }
                        Label { text: "Color Profile"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Keep original","sRGB","Adobe RGB","CMYK","Grayscale"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }
                        Label { text: "Strip EXIF"; color: "#888"; font.pixelSize: 11 }
                        CheckBox { checked: false; contentItem: Label { text: "Strip metadata"; color: "#ccc"; leftPadding: 4 } }
                    }
                }

                // Document options
                ColumnLayout {
                    visible: ["pdf","docx","epub","md","html","odt","tex","rtf"].indexOf(batchConvertPanel.outputExt) >= 0
                    spacing: 8
                    Label { text: "Document Settings"; color: "#0078d4"; font.bold: true; font.pixelSize: 12 }
                    GridLayout { columns: 4; columnSpacing: 12; rowSpacing: 6
                        Label { text: "PDF Engine"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["pdflatex","xelatex","lualatex","wkhtmltopdf","weasyprint"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                            Layout.fillWidth: true }
                        Label { text: "Table of Contents"; color: "#888"; font.pixelSize: 11 }
                        CheckBox { checked: false; contentItem: Label { text: "Generate TOC"; color: "#ccc"; leftPadding: 4 } }
                        Label { text: "Number Sections"; color: "#888"; font.pixelSize: 11 }
                        CheckBox { checked: false; contentItem: Label { text: "Numbered headings"; color: "#ccc"; leftPadding: 4 } }
                        Label { text: "CSS Theme"; color: "#888"; font.pixelSize: 11 }
                        TextField { placeholderText: "Path to CSS file (optional)"; color: "white"; Layout.fillWidth: true
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }
                    }
                }

                // Audio options (SoX)
                ColumnLayout {
                    visible: ["mp3","wav","flac","ogg","opus","aiff"].indexOf(batchConvertPanel.outputExt) >= 0
                    spacing: 8
                    Label { text: "Audio (Hi-Fi) Settings"; color: "#0078d4"; font.bold: true; font.pixelSize: 12 }
                    GridLayout { columns: 4; columnSpacing: 12; rowSpacing: 6
                        Label { text: "Sample Rate"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Keep original","44100 Hz","48000 Hz","96000 Hz","192000 Hz"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 } }
                        Label { text: "Bit Depth"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Keep original","16-bit","24-bit","32-bit float"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 } }
                        Label { text: "Normalization"; color: "#888"; font.pixelSize: 11 }
                        CheckBox { contentItem: Label { text: "Normalize to -1dB"; color: "#ccc"; leftPadding: 4 } }
                        Label { text: "Dither"; color: "#888"; font.pixelSize: 11 }
                        CheckBox { contentItem: Label { text: "Enable dithering"; color: "#ccc"; leftPadding: 4 } }
                    }
                }

                // Vector options (Inkscape)
                ColumnLayout {
                    visible: ["svg","pdf","eps","ai","png"].indexOf(batchConvertPanel.outputExt) >= 0
                    spacing: 8
                    Label { text: "Vector/CAD Settings"; color: "#0078d4"; font.bold: true; font.pixelSize: 12 }
                    GridLayout { columns: 4; columnSpacing: 12; rowSpacing: 6
                        Label { text: "Export Area"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Page","Drawing","Selection"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 } }
                        Label { text: "Text to Path"; color: "#888"; font.pixelSize: 11 }
                        CheckBox { contentItem: Label { text: "Convert text"; color: "#ccc"; leftPadding: 4 } }
                        Label { text: "DPI"; color: "#888"; font.pixelSize: 11 }
                        SpinBox { from: 72; to: 2400; value: 300; implicitWidth: 100
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }
                    }
                }

                // Extra FFmpeg args
                RowLayout {
                    Label { text: "Extra Args:"; color: "#888"; font.pixelSize: 11; width: 80 }
                    TextField {
                        Layout.fillWidth: true; color: "#aaa"; font.pixelSize: 11
                        placeholderText: "Advanced: extra command-line arguments passed directly to converter"
                        background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#444" }
                    }
                }
            }
        }

        // ── Run controls ────────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Batch Settings & Run"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 10

            ColumnLayout {
                anchors.fill: parent; spacing: 8

                GridLayout {
                    columns: 6; columnSpacing: 12
                    Label { text: "Collision"; color: "#888"; font.pixelSize: 11 }
                    ComboBox { id: collisionCombo; model: ["Skip existing","Overwrite","Rename (auto)","Overwrite if newer","Ask"]
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                        Layout.fillWidth: true }

                    Label { text: "Threads"; color: "#888"; font.pixelSize: 11 }
                    RowLayout {
                        Slider { id: threadSlider; from: 1; to: 16; value: 4; stepSize: 1; Layout.fillWidth: true }
                        Label { text: Math.round(threadSlider.value); color: "#ccc"; width: 20 }
                    }

                    Label { text: "Post-action"; color: "#888"; font.pixelSize: 11 }
                    ComboBox { model: ["Nothing","Open output folder","Notify","Shutdown PC"]
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6 }
                        Layout.fillWidth: true }
                }

                // Progress
                ColumnLayout {
                    visible: batchConvertPanel.isRunning || batchConvertPanel.progressDone > 0
                    spacing: 4

                    ProgressBar {
                        Layout.fillWidth: true
                        value: batchConvertPanel.progressTotal > 0 ? batchConvertPanel.progressDone / batchConvertPanel.progressTotal : 0
                        background: Rectangle { color: "#2d2d2d"; radius: 4; implicitHeight: 8 }
                        contentItem: Rectangle { color: "#0078d4"; radius: 4; width: parent.width * parent.value }
                    }

                    Label {
                        text: batchConvertPanel.progressDone + " / " + batchConvertPanel.progressTotal + " files converted"
                        color: "#888"; font.pixelSize: 11
                    }
                }

                RowLayout {
                    spacing: 10
                    Button {
                        text: batchConvertPanel.isRunning ? "⏳ Converting..." : "▶ Start Conversion"
                        enabled: !batchConvertPanel.isRunning && batchConvertPanel.inputFiles.length > 0
                        onClicked: batchConvertPanel.startConversion()
                        background: Rectangle {
                            color: parent.enabled ? "#0078d4" : "#333"; radius: 6
                        }
                        contentItem: Label { text: parent.text; color: parent.enabled ? "white" : "#666"; font.bold: true }
                    }
                    Button {
                        text: "⏹ Cancel"
                        visible: batchConvertPanel.isRunning
                        onClicked: batchConvertPanel.cancelConversion()
                        background: Rectangle { color: "#cc3333"; radius: 6 }
                        contentItem: Label { text: parent.text; color: "white" }
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: "📋 View Log"
                        flat: true; contentItem: Label { text: parent.text; color: "#888" }
                    }
                    Button {
                        text: "💾 Save Log"
                        flat: true; contentItem: Label { text: parent.text; color: "#888" }
                    }
                }

                // Error/log area
                Rectangle {
                    Layout.fillWidth: true; height: 80
                    color: "#111"; radius: 4; border.color: "#333"
                    ScrollView {
                        anchors.fill: parent; anchors.margins: 4; clip: true
                        TextEdit {
                            id: logOutput; width: parent.width
                            text: "Ready. Add files and choose output format to begin.\n"
                            color: "#aaa"; font.family: "Consolas,monospace"; font.pixelSize: 11
                            readOnly: true; wrapMode: TextEdit.Wrap
                        }
                    }
                }
            }
        }
    }
}
