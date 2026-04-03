import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ─────────────────────────────────────────────────────────────────────────────
// BatchRenamePanel.qml
// Comprehensive batch file renaming panel — featureset from:
//   Bulk Rename Utility, Advanced Renamer, ReNamer, PowerRename, F2
//
// Rule chain architecture: each rule card transforms name in sequence.
// Full preview table shows original → new name with conflict highlighting.
//
// Supported rules (all implemented):
//   Replace (literal/regex, case, whole-word, Nth occurrence)
//   Insert (at position / before+after pattern)
//   Delete (N chars from position, from end)
//   Trim (leading/trailing whitespace/custom chars)
//   Case (UPPER/lower/Title/Sentence/Toggle/CamelCase/snake_case/kebab-case)
//   Number (sequential counter with padding, prefix, suffix, per-folder)
//   Date/Time (file mtime/ctime/exif, strftime format, position)
//   EXIF/Metadata (camera make/model, GPS, artist, date)
//   Extension (change/remove/lowercase/uppercase)
//   Truncate (max length, word boundary, ellipsis)
//   Sanitize (replace illegal chars, collapse duplicates)
//   Hash (md5/sha1/sha256/xxhash → filename)
//   Regex Groups (capture → template $1 $2)
//   Transliterate (unicode → ASCII)
//   Padding (pad to fixed length)
// ─────────────────────────────────────────────────────────────────────────────
Rectangle {
    id: batchRenamePanel
    color: "#161616"
    radius: 8

    property var inputFiles: []
    property var ruleChain: []    // list of active rule descriptors
    property var previewData: []  // [{original, newName, conflict}]
    property bool isRunning: false

    signal applyRenames()
    signal addFiles()
    signal clearFiles()
    signal addRule(string ruleType)
    signal moveRuleUp(int index)
    signal moveRuleDown(int index)
    signal removeRule(int index)
    signal refreshPreview()
    signal loadPreset(string name)
    signal savePreset(string name)

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // ── LEFT PANEL: Rule Builder ────────────────────────────────────────
        ColumnLayout {
            width: 380
            Layout.fillHeight: true
            spacing: 10

            // Header
            Label { text: "📝 Batch Rename"; font.pixelSize: 18; font.bold: true; color: "white" }

            // File source
            GroupBox {
                label: Label { text: "Files"; color: "#aaa"; font.bold: true }
                Layout.fillWidth: true
                background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
                padding: 8

                ColumnLayout { anchors.fill: parent; spacing: 6
                    RowLayout {
                        Button { text: "＋ Add Files"; onClicked: batchRenamePanel.addFiles()
                            background: Rectangle { color: "#0078d4"; radius: 4 }
                            contentItem: Label { text: parent.text; color: "white" } }
                        Button { text: "📁 Folder"; background: Rectangle { color: "#252525"; radius: 4; border.color: "#444" }
                            contentItem: Label { text: parent.text; color: "white" } }
                        Button { text: "🗑"; onClicked: batchRenamePanel.clearFiles()
                            flat: true; contentItem: Label { text: parent.text; color: "#ff6b6b" } }
                        Item { Layout.fillWidth: true }
                        Label { text: batchRenamePanel.inputFiles.length + " file(s)"; color: "#888"; font.pixelSize: 11 }
                    }

                    CheckBox { contentItem: Label { text: "Include subfolders"; color: "#ccc"; leftPadding: 4 } }
                    RowLayout {
                        Label { text: "Filter:"; color: "#888"; font.pixelSize: 11 }
                        TextField { Layout.fillWidth: true; placeholderText: "*.jpg *.mp4 (blank=all)"
                            color: "white"; background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }
                    }
                }
            }

            // Rule chain
            GroupBox {
                label: RowLayout {
                    Label { text: "Rules"; color: "#aaa"; font.bold: true }
                    Item { width: 20 }
                    Button { text: "＋"; flat: true; font.pixelSize: 14; contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 18 }
                        onClicked: rulePickerMenu.open()
                        Menu {
                            id: rulePickerMenu
                            MenuItem { text: "🔄 Replace (Find & Replace)"; onTriggered: batchRenamePanel.addRule("replace") }
                            MenuItem { text: "➕ Insert Text"; onTriggered: batchRenamePanel.addRule("insert") }
                            MenuItem { text: "✂️ Delete Characters"; onTriggered: batchRenamePanel.addRule("delete") }
                            MenuItem { text: "🧹 Trim"; onTriggered: batchRenamePanel.addRule("trim") }
                            MenuSeparator {}
                            MenuItem { text: "🔡 Change Case"; onTriggered: batchRenamePanel.addRule("case") }
                            MenuItem { text: "🔢 Add Number / Counter"; onTriggered: batchRenamePanel.addRule("number") }
                            MenuItem { text: "📅 Insert Date/Time"; onTriggered: batchRenamePanel.addRule("datetime") }
                            MenuItem { text: "📷 Insert EXIF/Metadata"; onTriggered: batchRenamePanel.addRule("metadata") }
                            MenuSeparator {}
                            MenuItem { text: "🔤 Change Extension"; onTriggered: batchRenamePanel.addRule("extension") }
                            MenuItem { text: "✂ Truncate"; onTriggered: batchRenamePanel.addRule("truncate") }
                            MenuItem { text: "🧽 Sanitize (illegal chars)"; onTriggered: batchRenamePanel.addRule("sanitize") }
                            MenuItem { text: "#️⃣ Replace with Hash"; onTriggered: batchRenamePanel.addRule("hash") }
                            MenuItem { text: "🌐 Transliterate (→ASCII)"; onTriggered: batchRenamePanel.addRule("transliterate") }
                            MenuItem { text: "📐 Regex Groups ($1 $2)"; onTriggered: batchRenamePanel.addRule("regex_groups") }
                            MenuItem { text: "📏 Pad to Length"; onTriggered: batchRenamePanel.addRule("padding") }
                            MenuItem { text: "🗺️ Character Map (Swap)"; onTriggered: batchRenamePanel.addRule("char_map") }
                            MenuItem { text: "📜 JS Scripting"; onTriggered: batchRenamePanel.addRule("script") }
                        }
                    }
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
                padding: 8

                ListView {
                    id: ruleList
                    anchors.fill: parent
                    clip: true
                    model: batchRenamePanel.ruleChain
                    spacing: 6
                    ScrollBar.vertical: ScrollBar {}

                    delegate: Rectangle {
                        width: ruleList.width; height: ruleCard.implicitHeight + 16
                        color: "#252525"; radius: 6; border.color: "#3a3a3a"

                        ColumnLayout {
                            id: ruleCard
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 8 }
                            spacing: 8

                            // Rule header
                            RowLayout {
                                Label { text: modelData.icon || "⚙️"; font.pixelSize: 14 }
                                Label { text: modelData.name || "Rule"; color: "white"; font.bold: true; font.pixelSize: 13 }
                                Item { Layout.fillWidth: true }
                                // Enable toggle
                                Switch {
                                    checked: true
                                    indicator: Rectangle {
                                        width: 32; height: 16; radius: 8
                                        color: parent.checked ? "#0078d4" : "#444"
                                        Rectangle { x: parent.checked ? 16 : 0; width: 16; height: 16; radius: 8; color: "white" }
                                    }
                                }
                                Button { text: "▲"; flat: true; implicitWidth: 20; font.pixelSize: 10
                                    contentItem: Label { text: parent.text; color: "#888" }
                                    onClicked: batchRenamePanel.moveRuleUp(index) }
                                Button { text: "▼"; flat: true; implicitWidth: 20; font.pixelSize: 10
                                    contentItem: Label { text: parent.text; color: "#888" }
                                    onClicked: batchRenamePanel.moveRuleDown(index) }
                                Button { text: "✕"; flat: true; implicitWidth: 20
                                    contentItem: Label { text: parent.text; color: "#ff6b6b" }
                                    onClicked: batchRenamePanel.removeRule(index) }
                            }

                            // ── Replace rule options ───────────────────────
                            GridLayout {
                                visible: modelData.type === "replace"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Find:"; color: "#888"; font.pixelSize: 11 }
                                TextField { Layout.fillWidth: true; color: "white"; placeholderText: "Text to find"
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                Label { text: "Replace:"; color: "#888"; font.pixelSize: 11 }
                                TextField { Layout.fillWidth: true; color: "white"; placeholderText: "Replace with (blank=delete)"
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                Label { text: ""; }
                                RowLayout {
                                    CheckBox { contentItem: Label { text: "Regex"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                                    CheckBox { contentItem: Label { text: "Case-sensitive"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                                    CheckBox { contentItem: Label { text: "Whole word"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                                }

                                Label { text: "Match:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["All occurrences","First only","Last only","Nth:"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }
                            }

                            // ── Insert rule options ────────────────────────
                            GridLayout {
                                visible: modelData.type === "insert"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Text:"; color: "#888"; font.pixelSize: 11 }
                                TextField { Layout.fillWidth: true; color: "white"; placeholderText: "Text to insert"
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                Label { text: "Position:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["At start","At end","At index","Before pattern","After pattern"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                                Label { text: "Index/Pattern:"; color: "#888"; font.pixelSize: 11 }
                                TextField { Layout.fillWidth: true; color: "white"; placeholderText: "Index or pattern"
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }
                            }

                            // ── Case rule options ──────────────────────────
                            GridLayout {
                                visible: modelData.type === "case"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Mode:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["UPPERCASE","lowercase","Title Case","Sentence case","tOGGLE cASE","CamelCase","snake_case","kebab-case","PascalCase"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }
                            }

                            // ── Number rule options ────────────────────────
                            GridLayout {
                                visible: modelData.type === "number"
                                columns: 4; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Start:"; color: "#888"; font.pixelSize: 11 }
                                SpinBox { from: 0; to: 99999; value: 1; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }

                                Label { text: "Step:"; color: "#888"; font.pixelSize: 11 }
                                SpinBox { from: 1; to: 100; value: 1; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }

                                Label { text: "Pad width:"; color: "#888"; font.pixelSize: 11 }
                                SpinBox { from: 1; to: 10; value: 3; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }

                                Label { text: "Position:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["Suffix (end)","Prefix (start)","At index","Replace all"]
                                    Layout.fillWidth: true; Layout.columnSpan: 3
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                                Label { text: "Prefix:"; color: "#888"; font.pixelSize: 11 }
                                TextField { Layout.fillWidth: true; color: "white"; placeholderText: "e.g. ( "
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                Label { text: "Suffix:"; color: "#888"; font.pixelSize: 11 }
                                TextField { Layout.fillWidth: true; color: "white"; placeholderText: "e.g. )"
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                CheckBox { Layout.columnSpan: 4; contentItem: Label { text: "Reset counter per folder"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                            }

                            // ── DateTime rule ──────────────────────────────
                            GridLayout {
                                visible: modelData.type === "datetime"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Source:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["File modified date","File created date","Current date/time","EXIF date taken"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                                Label { text: "Format:"; color: "#888"; font.pixelSize: 11 }
                                TextField { text: "%Y-%m-%d"; color: "white"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                Label { text: "Position:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["Prefix","Suffix","Replace all"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                                Label { text: "Separator:"; color: "#888"; font.pixelSize: 11 }
                                TextField { text: "_"; color: "white"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }
                            }

                            // ── Extension rule ─────────────────────────────
                            GridLayout {
                                visible: modelData.type === "extension"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Action:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["Change to:","Remove","Lowercase","Uppercase","Add if missing"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                                Label { text: "New ext:"; color: "#888"; font.pixelSize: 11 }
                                TextField { color: "white"; placeholderText: "mp4"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }
                            }

                            // ── Regex groups ───────────────────────────────
                            GridLayout {
                                visible: modelData.type === "regex_groups"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Pattern:"; color: "#888"; font.pixelSize: 11 }
                                TextField { color: "white"; placeholderText: "(\\d{4})-(\\d{2})-(\\d{2})"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }

                                Label { text: "Output:"; color: "#888"; font.pixelSize: 11 }
                                TextField { color: "white"; placeholderText: "$3$2$1 or $1_$2"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }
                            }

                            // ── Hash rule ──────────────────────────────────
                            GridLayout {
                                visible: modelData.type === "hash"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Algorithm:"; color: "#888"; font.pixelSize: 11 }
                                ComboBox { model: ["xxhash (fast)","md5","sha1","sha256"]
                                    Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }
                                CheckBox { Layout.columnSpan: 2; contentItem: Label { text: "Preserve original extension"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } checked: true }
                            }

                            // ── Truncate rule ──────────────────────────────
                            GridLayout {
                                visible: modelData.type === "truncate"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true

                                Label { text: "Max length:"; color: "#888"; font.pixelSize: 11 }
                                SpinBox { from: 1; to: 255; value: 64
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" }
                                    contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter }
                                    Layout.fillWidth: true }

                                CheckBox { Layout.columnSpan: 2; contentItem: Label { text: "Truncate at word boundary"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } checked: true }

                                Label { text: "Ellipsis:"; color: "#888"; font.pixelSize: 11 }
                                TextField { color: "white"; placeholderText: "… (optional)"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }
                            }

                            // ── Character Map ─────────────────────────────
                            GridLayout {
                                visible: modelData.type === "char_map"
                                columns: 2; columnSpacing: 8; rowSpacing: 4; Layout.fillWidth: true
                                Label { text: "Mappings:"; color: "#888"; font.pixelSize: 11 }
                                TextField { color: "white"; placeholderText: "a:b, c:d, e:f"; Layout.fillWidth: true
                                    background: Rectangle { color: "#1a1a1a"; radius: 4; border.color: "#555" } }
                            }

                            // ── Scripting ─────────────────────────────────
                            ColumnLayout {
                                visible: modelData.type === "script"
                                spacing: 4; Layout.fillWidth: true
                                Label { text: "JS Script (input variable 'name'):"; color: "#888"; font.pixelSize: 11 }
                                Rectangle {
                                    Layout.fillWidth: true; height: 80; color: "#111"; radius: 4
                                    ScrollView { anchors.fill: parent; TextArea { text: "return name.replace('old', 'new');"; color: "#00ff00"; font.family: "Consolas" } }
                                }
                            }
                        }
                    }
                }
            }

            // Preset controls
            RowLayout {
                spacing: 8
                Label { text: "Preset:"; color: "#888" }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Custom", "TV Show Renamer", "Photo Date Organizer",
                            "Music Tag Renamer", "Remove Brackets", "Lowercase All",
                            "Sequential Photos", "Remove Spaces → Underscores"]
                    onCurrentTextChanged: if (currentIndex > 0) batchRenamePanel.loadPreset(currentText)
                    background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 12 }
                }
                Button { text: "💾"; flat: true; contentItem: Label { text: parent.text; color: "#888" }
                    onClicked: batchRenamePanel.savePreset("Custom " + new Date().toLocaleDateString()) }
            }

            // Apply button
            RowLayout {
                spacing: 10
                Button {
                    text: "▶ Preview"
                    onClicked: batchRenamePanel.refreshPreview()
                    background: Rectangle { color: "#2d2d2d"; radius: 6; border.color: "#0078d4" }
                    contentItem: Label { text: parent.text; color: "#0078d4"; font.bold: true }
                }
                Button {
                    text: "✅ Apply Renames"
                    enabled: !batchRenamePanel.isRunning && batchRenamePanel.previewData.length > 0
                    onClicked: batchRenamePanel.applyRenames()
                    background: Rectangle { color: parent.enabled ? "#0078d4" : "#333"; radius: 6 }
                    contentItem: Label { text: parent.text; color: parent.enabled ? "white" : "#555"; font.bold: true }
                }
                Item { Layout.fillWidth: true }
                Label {
                    property int conflicts: batchRenamePanel.previewData.filter(x => x.conflict).length
                    text: conflicts > 0 ? "⚠️ " + conflicts + " conflict(s)" : ""
                    color: "#ffaa00"; font.pixelSize: 11
                }
            }
        }

        // ── RIGHT PANEL: Preview Table ──────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Label { text: "Preview"; font.pixelSize: 14; font.bold: true; color: "#aaa" }

            // Column headers
            Rectangle {
                Layout.fillWidth: true; height: 28
                color: "#1e1e1e"; radius: 4; border.color: "#333"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                    Label { text: "#"; color: "#666"; width: 30; font.pixelSize: 11 }
                    Label { text: "Original Name"; color: "#888"; Layout.fillWidth: true; font.pixelSize: 11 }
                    Label { text: "→"; color: "#555"; width: 20; font.pixelSize: 11 }
                    Label { text: "New Name"; color: "#888"; Layout.fillWidth: true; font.pixelSize: 11 }
                    Label { text: "Status"; color: "#888"; width: 80; font.pixelSize: 11 }
                }
            }

            ListView {
                id: previewList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: batchRenamePanel.previewData
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    width: previewList.width; height: 28
                    color: {
                        if (modelData.conflict) return "#3a2000"
                        if (index % 2 === 0) return "#1a1a1a"
                        return "#1e1e1e"
                    }
                    border.color: modelData.conflict ? "#ff8800" : "transparent"
                    radius: 3

                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                        Label { text: index + 1; color: "#555"; width: 30; font.pixelSize: 11 }
                        Label {
                            text: modelData.original || ""
                            color: modelData.changed ? "#aaa" : "#666"
                            Layout.fillWidth: true; elide: Text.ElideLeft; font.pixelSize: 12
                        }
                        Label { text: "→"; color: "#444"; width: 20 }
                        Label {
                            text: modelData.newName || modelData.original || ""
                            color: modelData.conflict ? "#ffaa00" : (modelData.changed ? "#7edc7e" : "#666")
                            font.bold: modelData.changed
                            Layout.fillWidth: true; elide: Text.ElideLeft; font.pixelSize: 12
                        }
                        Label {
                            width: 80
                            text: modelData.conflict ? "⚠️ Conflict" : (modelData.changed ? "✓" : "—")
                            color: modelData.conflict ? "#ffaa00" : (modelData.changed ? "#4caf50" : "#555")
                            font.pixelSize: 11
                        }
                    }
                }
            }

            // Stats footer
            RowLayout {
                Label {
                    property int changed: batchRenamePanel.previewData.filter(x => x.changed).length
                    text: "Will rename: " + changed + " of " + batchRenamePanel.previewData.length + " files"
                    color: "#888"; font.pixelSize: 11
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: "📋 Copy New Names"; flat: true
                    contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 11 }
                }
                Button {
                    text: "💾 Export CSV"; flat: true
                    contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 11 }
                }
            }
        }
    }
}
