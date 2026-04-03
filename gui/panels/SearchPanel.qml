import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ─────────────────────────────────────────────────────────────────────────────
// SearchPanel.qml
// Comprehensive filename + content search panel — featureset from:
//   Everything (voidtools): instant filename search, regex, size/date filters
//   AgentRansack/FileLocator: GUI grep with context, binary detect
//   grepWin: PCRE2 regex search+replace with preview
//   dnGrep: Everything + content, archive search, find+replace
//   ripgrep: fastest content search, .gitignore aware
//
// Modes: Filename Search | Content Search | Find & Replace
// ─────────────────────────────────────────────────────────────────────────────
Rectangle {
    id: searchPanel
    color: "#161616"
    radius: 8

    property string searchQuery: ""
    property string contentQuery: ""
    property string replaceWith: ""
    property int resultsCount: 0
    property int matchesCount: 0
    property bool isSearching: false
    property bool isReplacing: false
    property var searchRoots: []
    property var results: []           // [{path,size,modified,matches:[{line,col,text}]}]
    property int activeMode: 0        // 0=Filename, 1=Content, 2=Replace

    signal startSearch()
    signal cancelSearch()
    signal startReplace()
    signal openFile(string path)
    signal openFolder(string path)
    signal addRoot()
    signal clearResults()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // ── Tab bar ─────────────────────────────────────────────────────────
        RowLayout {
            spacing: 0

            Label { text: "🔍 Search"; font.pixelSize: 18; font.bold: true; color: "white" }
            Item { width: 20 }

            Repeater {
                model: ["📁 Filename Search", "📄 Content Search", "🔄 Find & Replace", "🧠 Semantic Search"]
                Rectangle {
                    width: implicitLabel.implicitWidth + 24; height: 34
                    color: searchPanel.activeMode === index ? "#0078d4" : "#252525"
                    radius: 4
                    border.color: searchPanel.activeMode === index ? "#0078d4" : "#444"
                    Label { id: implicitLabel; anchors.centerIn: parent; text: modelData; color: "white"; font.pixelSize: 12 }
                    MouseArea { anchors.fill: parent; onClicked: searchPanel.activeMode = index }
                }
            }

            Item { Layout.fillWidth: true }

            Button { text: "🗑 Clear Results"; flat: true
                contentItem: Label { text: parent.text; color: "#888" }
                onClicked: searchPanel.clearResults() }
        }

        // ── Quick Filter Bar (Everything-like) ──────────────────────────────
        RowLayout {
            spacing: 8
            Label { text: "Quick Filter:"; color: "#888"; font.pixelSize: 11 }
            Repeater {
                model: ["Everything", "Audio", "Compressed", "Document", "Executable", "Folder", "Image", "Video"]
                Button {
                    text: modelData
                    flat: true
                    contentItem: Label {
                        text: parent.text; color: index === 0 ? "#0078d4" : "#888"
                        font.bold: index === 0; font.pixelSize: 11
                    }
                }
            }
            Item { Layout.fillWidth: true }
        }

        // ── Search roots ─────────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Search in"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 8

            RowLayout {
                anchors.fill: parent; spacing: 8

                ListView {
                    id: rootList
                    Layout.fillWidth: true; height: 26; orientation: ListView.Horizontal; spacing: 6; clip: true
                    model: searchPanel.searchRoots
                    delegate: Rectangle {
                        height: 24; width: rootLabel.implicitWidth + 24
                        color: "#2d2d2d"; radius: 4; border.color: "#555"
                        RowLayout { anchors.fill: parent; anchors.leftMargin: 8; spacing: 4
                            Label { id: rootLabel; text: modelData; color: "#ccc"; font.pixelSize: 11 }
                            Label { text: "✕"; color: "#666"; font.pixelSize: 10
                                MouseArea { anchors.fill: parent; onClicked: { /* remove */ } } }
                        }
                    }
                }

                Button { text: "＋ Add Folder"; onClicked: searchPanel.addRoot()
                    background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                    contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 11 } }

                CheckBox { contentItem: Label { text: "Recursive"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } checked: true }
                Label { text: "Depth:"; color: "#888"; font.pixelSize: 11 }
                SpinBox { from: -1; to: 99; value: -1; implicitWidth: 70
                    background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                    contentItem: TextInput { text: parent.value === -1 ? "∞" : parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter }
                    textFromValue: function(v) { return v === -1 ? "∞" : v } }
            }
        }

        // ── Main query area ──────────────────────────────────────────────────
        GroupBox {
            label: Label { text: searchPanel.activeMode === 0 ? "Filename Query" : searchPanel.activeMode === 1 ? "Content Query" : searchPanel.activeMode === 2 ? "Find & Replace" : "Semantic Query (Describe the image)"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 10

            ColumnLayout {
                anchors.fill: parent; spacing: 10

                // ── Filename/Content/Semantic query row ─────────────────────────────
                RowLayout {
                    spacing: 8; Layout.fillWidth: true

                    Rectangle {
                        Layout.fillWidth: true; height: 36
                        color: "#1a1a1a"; radius: 6; border.color: qField.activeFocus ? "#0078d4" : "#444"

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 8; spacing: 8
                            Label { text: searchPanel.activeMode === 0 ? "🔍" : searchPanel.activeMode === 1 ? "📄" : searchPanel.activeMode === 2 ? "🔍" : "🧠"; color: "#888" }
                            TextInput {
                                id: qField
                                Layout.fillWidth: true
                                text: searchPanel.searchQuery
                                color: "white"; font.pixelSize: 14
                                placeholderText: searchPanel.activeMode === 0 ? "Filename or path (regex / wildcard / fuzzy)..." :
                                                 searchPanel.activeMode === 1 ? "Search in file contents..." :
                                                 searchPanel.activeMode === 2 ? "Find (in file contents)..." :
                                                 "Describe the image you're looking for (e.g. 'a dog playing in a park at sunset')..."
                                onTextChanged: searchPanel.searchQuery = text
                                onAccepted: searchPanel.startSearch()
                            }
                            // Live clear button
                            Label { text: "✕"; color: "#666"; visible: qField.text.length > 0
                                MouseArea { anchors.fill: parent; onClicked: { qField.text = ""; searchPanel.searchQuery = "" } } }
                        }
                    }

                    Button {
                        text: searchPanel.isSearching ? "⏳" : "▶ Search"
                        onClicked: searchPanel.isSearching ? searchPanel.cancelSearch() : searchPanel.startSearch()
                        background: Rectangle {
                            color: searchPanel.isSearching ? "#cc3333" : "#0078d4"; radius: 6
                        }
                        contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                    }
                }

                // Replace row (mode 2 only)
                RowLayout {
                    visible: searchPanel.activeMode === 2
                    spacing: 8; Layout.fillWidth: true

                    Rectangle {
                        Layout.fillWidth: true; height: 36
                        color: "#1a1a1a"; radius: 6; border.color: replaceField.activeFocus ? "#f0a070" : "#444"
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 8
                            Label { text: "🔄"; color: "#888" }
                            TextInput {
                                id: replaceField; Layout.fillWidth: true
                                text: searchPanel.replaceWith; color: "white"; font.pixelSize: 14
                                placeholderText: "Replace with... (use $1 $2 for regex groups)"
                                onTextChanged: searchPanel.replaceWith = text
                            }
                        }
                    }

                    Button {
                        text: "✅ Replace All"
                        onClicked: searchPanel.startReplace()
                        background: Rectangle { color: "#cc6600"; radius: 6 }
                        contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                    }
                }

                // ── Match options row ──────────────────────────────────────
                Flow { Layout.fillWidth: true; spacing: 8

                    // Match mode
                    Label { text: "Mode:"; color: "#888"; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
                    ButtonGroup { id: matchModeGroup }
                    Repeater {
                        model: ["Literal","Wildcard","Regex","Fuzzy"]
                        RadioButton {
                            text: modelData; ButtonGroup.group: matchModeGroup; checked: index === 0
                            contentItem: Label { text: parent.text; color: parent.checked ? "#0078d4" : "#888"; font.pixelSize: 11 }
                        }
                    }

                    Rectangle { width: 1; height: 16; color: "#444"; anchors.verticalCenter: parent.verticalCenter }

                    CheckBox { id: caseSensitiveCheck; contentItem: Label { text: "Case-sensitive"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                    CheckBox { id: wholeWordCheck; contentItem: Label { text: "Whole word"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                    CheckBox { id: fullPathCheck; contentItem: Label { text: "Match full path"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                    CheckBox { id: invertCheck; contentItem: Label { text: "Invert match"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                    CheckBox {
                        visible: searchPanel.activeMode >= 1
                        contentItem: Label { text: "Search archives"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 }
                    }
                    CheckBox {
                        id: gitignoreCheck; checked: true
                        visible: searchPanel.activeMode >= 1
                        contentItem: Label { text: "Respect .gitignore"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 }
                    }
                }

                // ── Content search options (mode 1+2) ────────────────────
                ColumnLayout {
                    visible: searchPanel.activeMode >= 1
                    spacing: 6; Layout.fillWidth: true

                    RowLayout {
                        spacing: 8

                        Label { text: "Context lines:"; color: "#888"; font.pixelSize: 11 }
                        Label { text: "Before:"; color: "#666"; font.pixelSize: 11 }
                        SpinBox { from: 0; to: 20; value: 0; implicitWidth: 70
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }
                        Label { text: "After:"; color: "#666"; font.pixelSize: 11 }
                        SpinBox { from: 0; to: 20; value: 0; implicitWidth: 70
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }

                        Rectangle { width: 1; height: 16; color: "#444" }

                        Label { text: "Threads:"; color: "#888"; font.pixelSize: 11 }
                        SpinBox { from: 1; to: 32; value: 4; implicitWidth: 70
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: TextInput { text: parent.value; color: "white"; horizontalAlignment: Qt.AlignHCenter } }

                        Label { text: "Encoding:"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["Auto-detect","UTF-8","UTF-16","ANSI","OEM (850)","ISO-8859-1"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                        CheckBox { contentItem: Label { text: "Multiline"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                        CheckBox { contentItem: Label { text: "Hex search"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }

                        Item { Layout.fillWidth: true }
                        Label { text: "Max file size:"; color: "#888"; font.pixelSize: 11 }
                        ComboBox { model: ["100 MB","10 MB","1 MB","500 KB","No limit"]
                            background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }
                    }
                }
            }
        }

        // ── File Filters ─────────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "File Filters"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 10
            // Collapsed by default
            property bool collapsed: true

            ColumnLayout {
                anchors.fill: parent; spacing: 8
                visible: !parent.collapsed

                GridLayout {
                    columns: 6; columnSpacing: 12; rowSpacing: 6; Layout.fillWidth: true

                    Label { text: "Include ext:"; color: "#888"; font.pixelSize: 11 }
                    TextField { Layout.fillWidth: true; placeholderText: "cpp hpp py js (space-sep)"; color: "white"
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    Label { text: "Exclude ext:"; color: "#888"; font.pixelSize: 11 }
                    TextField { Layout.fillWidth: true; placeholderText: "exe dll bin obj"; color: "white"
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    Label { text: "Exclude dirs:"; color: "#888"; font.pixelSize: 11 }
                    TextField { Layout.fillWidth: true; placeholderText: "node_modules .git build dist"; color: "white"
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    // Size filter
                    Label { text: "Min size:"; color: "#888"; font.pixelSize: 11 }
                    TextField { placeholderText: "1K / 1M / 1G"; color: "white"; Layout.fillWidth: true
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    Label { text: "Max size:"; color: "#888"; font.pixelSize: 11 }
                    TextField { placeholderText: "100M"; color: "white"; Layout.fillWidth: true
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    Label { text: "Glob exclude:"; color: "#888"; font.pixelSize: 11 }
                    TextField { Layout.fillWidth: true; placeholderText: "*.bak *.tmp ~*"; color: "white"
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    // Date filter
                    Label { text: "Modified after:"; color: "#888"; font.pixelSize: 11 }
                    TextField { placeholderText: "YYYY-MM-DD"; color: "white"; Layout.fillWidth: true
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    Label { text: "Modified before:"; color: "#888"; font.pixelSize: 11 }
                    TextField { placeholderText: "YYYY-MM-DD"; color: "white"; Layout.fillWidth: true
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" } }

                    Label { text: ""; }
                    RowLayout {
                        CheckBox { contentItem: Label { text: "Skip hidden"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                        CheckBox { contentItem: Label { text: "Skip system"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                        CheckBox { contentItem: Label { text: "Files only"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                        CheckBox { contentItem: Label { text: "Dirs only"; color: "#ccc"; leftPadding: 4; font.pixelSize: 11 } }
                    }
                }
            }

            // Collapse toggle
            Rectangle {
                anchors { bottom: parent.top; right: parent.right; rightMargin: 0 }
                visible: false // Handled by label click
            }
        }

        // ── Results area ──────────────────────────────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6

            // Results header
            RowLayout {
                Label {
                    text: searchPanel.isSearching ? "Searching..." :
                          searchPanel.resultsCount + " file(s) found" +
                          (searchPanel.matchesCount > 0 ? " · " + searchPanel.matchesCount + " match(es)" : "")
                    color: searchPanel.isSearching ? "#0078d4" : "#aaa"; font.pixelSize: 13
                }
                Item { Layout.fillWidth: true }

                // Semantic threshold slider
                RowLayout {
                    visible: searchPanel.activeMode === 3
                    Label { text: "Threshold:"; color: "#888"; font.pixelSize: 11 }
                    Slider { from: 0; to: 1; value: 0.22; implicitWidth: 100 }
                }

                // Sort/group options
                Label { text: "Sort:"; color: "#888"; font.pixelSize: 11 }
                ComboBox { model: ["Relevance","Path","Name","Size","Date modified","Match count"]
                    background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                Label { text: "Group:"; color: "#888"; font.pixelSize: 11 }
                ComboBox { model: ["None","By folder","By extension","By date"]
                    background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                    contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 6; font.pixelSize: 11 } }

                Button { text: "📋 Copy paths"; flat: true; contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 11 } }
                Button { text: "💾 Export CSV"; flat: true; contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 11 } }
                Button { text: "📂 Open folder"; flat: true; contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 11 } }
            }

            // Semantic Grid Results
            GridView {
                id: semanticResults
                visible: searchPanel.activeMode === 3
                Layout.fillWidth: true; Layout.fillHeight: true
                clip: true; model: searchPanel.results
                cellWidth: 180; cellHeight: 180
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    width: 170; height: 170
                    color: "#1e1e1e"; radius: 6; border.color: "#333"

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 4; spacing: 4
                        
                        // Thumbnail
                        Rectangle {
                            Layout.fillWidth: true; Layout.fillHeight: true; color: "#0d0d0d"; radius: 4
                            Image {
                                anchors.fill: parent; anchors.margins: 2
                                source: "image:/thumb/" + (modelData.path || "")
                                fillMode: Image.PreserveAspectFit
                            }
                            // Relevance score badge
                            Rectangle {
                                anchors.top: parent.top; anchors.right: parent.right; anchors.margins: 4
                                width: 40; height: 16; radius: 8; color: "#0078d4dd"
                                Label { anchors.centerIn: parent; text: Math.round((modelData.score || 0)*100)+"%"; color: "white"; font.pixelSize: 10 }
                            }
                        }

                        Label {
                            text: {
                                var p = modelData.path || ""
                                return p.split("/").pop() || p.split("\\").pop() || p
                            }
                            color: "#ccc"; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onDoubleClicked: searchPanel.openFile(modelData.path || "")
                    }
                }
            }

            // Results list (filename search)
            ListView {
                id: filenameResults
                visible: searchPanel.activeMode === 0
                Layout.fillWidth: true; Layout.fillHeight: true
                clip: true; model: searchPanel.results
                ScrollBar.vertical: ScrollBar {}

                // Column headers
                header: Rectangle {
                    width: filenameResults.width; height: 26
                    color: "#1e1e1e"; border.color: "#333"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                        Label { text: "Name"; color: "#666"; Layout.fillWidth: true; font.pixelSize: 11; font.bold: true }
                        Label { text: "Path"; color: "#666"; Layout.preferredWidth: 300; font.pixelSize: 11; font.bold: true }
                        Label { text: "Size"; color: "#666"; Layout.preferredWidth: 80; font.pixelSize: 11; font.bold: true }
                        Label { text: "Modified"; color: "#666"; Layout.preferredWidth: 140; font.pixelSize: 11; font.bold: true }
                    }
                }

                delegate: Rectangle {
                    width: filenameResults.width; height: 28
                    color: index % 2 === 0 ? "#1a1a1a" : "#1e1e1e"
                    HoverHandler { id: rowHover }

                    Rectangle { anchors.fill: parent; color: "#0078d430"; visible: rowHover.hovered; radius: 2 }

                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                        Label {
                            text: {
                                var p = modelData.path || ""
                                return p.split("/").pop() || p.split("\\").pop() || p
                            }
                            color: "#e0e0e0"; Layout.fillWidth: true; elide: Text.ElideRight; font.pixelSize: 12
                        }
                        Label {
                            text: {
                                var p = modelData.path || ""
                                var parts = p.split("/")
                                parts.pop(); return parts.join("/") || p
                            }
                            color: "#888"; Layout.preferredWidth: 300; elide: Text.ElideLeft; font.pixelSize: 11
                        }
                        Label { text: modelData.sizeStr || ""; color: "#888"; Layout.preferredWidth: 80; font.pixelSize: 11 }
                        Label { text: modelData.modifiedStr || ""; color: "#888"; Layout.preferredWidth: 140; font.pixelSize: 11 }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onDoubleClicked: searchPanel.openFile(modelData.path || "")
                        onClicked: function(m) {
                            if (m.button === Qt.RightButton) resultContextMenu.popup()
                        }
                        Menu {
                            id: resultContextMenu
                            MenuItem { text: "Open File"; onTriggered: searchPanel.openFile(modelData.path || "") }
                            MenuItem { text: "Open Containing Folder"; onTriggered: searchPanel.openFolder(modelData.path || "") }
                            MenuSeparator {}
                            MenuItem { text: "Copy Path"; onTriggered: { /* clipboard */ } }
                            MenuItem { text: "Copy Filename"; onTriggered: { /* clipboard */ } }
                        }
                    }
                }
            }

            // Content search results (expand/collapse per-file matches)
            ListView {
                id: contentResults
                visible: searchPanel.activeMode >= 1
                Layout.fillWidth: true; Layout.fillHeight: true
                clip: true; model: searchPanel.results
                ScrollBar.vertical: ScrollBar {}

                delegate: Column {
                    width: contentResults.width

                    // File header row
                    Rectangle {
                        width: parent.width; height: 32
                        color: fileExpanded.checked ? "#252525" : "#1e1e1e"
                        border.color: "#333"

                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                            Label { text: fileExpanded.checked ? "▼" : "▶"; color: "#888"; width: 16 }
                            Label { text: "📄"; font.pixelSize: 14 }
                            Label { text: modelData.path || ""; color: "#e0e0e0"; Layout.fillWidth: true; elide: Text.ElideLeft; font.pixelSize: 12 }
                            Rectangle {
                                width: matchBadge.implicitWidth + 12; height: 18; radius: 9
                                color: "#0078d430"; border.color: "#0078d4"
                                Label { id: matchBadge; text: (modelData.matchCount || 0) + " match(es)"; color: "#0078d4"; font.pixelSize: 10; anchors.centerIn: parent }
                            }
                        }

                        MouseArea { anchors.fill: parent; onClicked: fileExpanded.checked = !fileExpanded.checked }

                        CheckBox { id: fileExpanded; visible: false; checked: false }
                    }

                    // Match lines (collapsible)
                    Repeater {
                        model: fileExpanded.checked ? (modelData.matches || []) : []

                        Rectangle {
                            width: contentResults.width; height: 24
                            color: "#111"; border.color: "#222"

                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 40; anchors.rightMargin: 8
                                // Line number
                                Label {
                                    text: (modelData.lineNumber || 0) + ":"
                                    color: "#0078d4"; font.pixelSize: 11; font.family: "Consolas,monospace"; width: 48
                                }
                                // Match line with highlighted match
                                Label {
                                    text: modelData.lineText || ""
                                    color: "#aaa"; font.pixelSize: 11; font.family: "Consolas,monospace"
                                    elide: Text.ElideRight; Layout.fillWidth: true
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onDoubleClicked: searchPanel.openFile((modelData.path || "") + ":" + (modelData.lineNumber || 0))
                            }
                        }
                    }
                }
            }
        }

        // ── Status bar ───────────────────────────────────────────────────────
        RowLayout {
            Rectangle { width: 8; height: 8; radius: 4; color: searchPanel.isSearching ? "#4caf50" : "#444" }
            Label {
                text: searchPanel.isSearching ? "Searching..." : "Ready"
                color: "#666"; font.pixelSize: 11
            }
            Item { Layout.fillWidth: true }
            Label {
                text: searchPanel.resultsCount > 0 ? searchPanel.resultsCount + " results" : ""
                color: "#666"; font.pixelSize: 11
            }
        }
    }
}
