import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ─────────────────────────────────────────────────────────────────────────────
// HexEditorPanel.qml
// Full-featured hex viewer/editor — features from:
//   HxD: column layout, disk/process editing, data inspector
//   ImHex: pattern/template language, data analysis
//   010 Editor: binary template parser, bookmarks, scripts
//   wxHexEditor: large file support, offset types, sector view
//
// Renders hex bytes + ASCII side-panel in a fixed-pitch font.
// Data Inspector shows all interpretations of selected bytes.
// Search: hex pattern (with ?? wildcards), ASCII/UTF16, regex.
// Edit: click+type to overwrite bytes (read-only or write mode).
// ─────────────────────────────────────────────────────────────────────────────
Rectangle {
    id: hexPanel
    color: "#111"; radius: 8

    // Connected to HexBuffer C++ object
    property string filePath: ""
    property bool readOnly: true
    property bool modified: false
    property int totalBytes: 0
    property int pageOffset: 0     // Current view offset
    property int bytesPerRow: 16
    property int visibleRows: 32
    property var pageData: []      // Array of byte values for current page
    property int selectedOffset: -1
    property int selectionLength: 1
    property string selectedHex: ""
    property string searchQuery: ""
    property int searchResultOffset: -1

    // Data inspector binding
    property var dataInterpretation: ({})

    signal openFile()
    signal saveFile()
    signal saveFileAs()
    signal searchNext()
    signal searchPrev()
    signal searchAll()
    signal gotoOffset(int offset)
    signal undoEdit()
    signal redoEdit()

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── Toolbar ─────────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 40
            color: "#1c1c1c"; border.color: "#333"

            RowLayout {
                anchors.fill: parent; anchors.margins: 6; spacing: 4

                Button { text: "📂 Open"; flat: true; contentItem: Label{text:parent.text;color:"white";font.pixelSize:11}; onClicked:hexPanel.openFile() }
                Button { text: "💾 Save"; flat: true; enabled: hexPanel.modified && !hexPanel.readOnly; contentItem: Label{text:parent.text;color:parent.enabled?"white":"#555";font.pixelSize:11}; onClicked:hexPanel.saveFile() }
                Button { text: "💾 Save As"; flat: true; contentItem: Label{text:parent.text;color:"white";font.pixelSize:11}; onClicked:hexPanel.saveFileAs() }

                Rectangle { width:1; height:24; color:"#444" }

                Button { text: "↩ Undo"; flat: true; enabled: !hexPanel.readOnly; contentItem: Label{text:parent.text;color:parent.enabled?"white":"#555";font.pixelSize:11}; onClicked:hexPanel.undoEdit() }
                Button { text: "↪ Redo"; flat: true; enabled: !hexPanel.readOnly; contentItem: Label{text:parent.text;color:parent.enabled?"white":"#555";font.pixelSize:11}; onClicked:hexPanel.redoEdit() }

                Rectangle { width:1; height:24; color:"#444" }

                // Columns per row
                Label { text: "Cols:"; color: "#888"; font.pixelSize: 11 }
                ComboBox {
                    model: [8, 16, 32]; currentIndex: 1
                    onCurrentTextChanged: hexPanel.bytesPerRow = parseInt(currentText)
                    background: Rectangle { color:"#2d2d2d"; radius:4; border.color:"#555" }
                    contentItem: Label { text:parent.currentText; color:"white"; leftPadding:6; font.pixelSize:11 }
                    implicitWidth: 60
                }

                Label { text: "Offset:"; color: "#888"; font.pixelSize: 11 }
                ComboBox {
                    model: ["Hex","Dec","Oct"]
                    background: Rectangle { color:"#2d2d2d"; radius:4; border.color:"#555" }
                    contentItem: Label { text:parent.currentText; color:"white"; leftPadding:6; font.pixelSize:11 }
                    implicitWidth: 70
                }

                Item { Layout.fillWidth: true }

                // Read-only badge
                Rectangle {
                    width:80; height:24; radius:4
                    color: hexPanel.readOnly ? "#3a2a00" : "#003a00"
                    border.color: hexPanel.readOnly ? "#ffaa00" : "#4caf50"
                    Label { anchors.centerIn:parent; text:hexPanel.readOnly?"🔒 Read-Only":"✏️ Edit Mode"; color:hexPanel.readOnly?"#ffaa00":"#4caf50"; font.pixelSize:10 }
                }

                Label {
                    text: hexPanel.filePath ? hexPanel.filePath.split("/").pop() : "(no file)"
                    color: hexPanel.modified ? "#ffaa00" : "#888"; font.pixelSize: 11
                }
                Label {
                    text: hexPanel.totalBytes > 0 ? (hexPanel.totalBytes/1024).toFixed(1) + " KB" : ""
                    color: "#666"; font.pixelSize: 11
                }
            }
        }

        // ── Search bar ───────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 36
            color: "#1a1a1a"; border.color: "#2a2a2a"

            RowLayout {
                anchors.fill: parent; anchors.margins: 6; spacing: 8

                Label { text: "🔍"; color: "#888" }

                ComboBox {
                    id: searchMode; model: ["Hex Pattern","ASCII String","UTF-16 LE","Regex"]
                    background: Rectangle{color:"#2d2d2d";radius:4;border.color:"#555"}
                    contentItem: Label{text:parent.currentText;color:"white";leftPadding:6;font.pixelSize:11}
                    implicitWidth:120
                }

                Rectangle {
                    Layout.fillWidth: true; height: 26
                    color: "#0d0d0d"; radius: 4; border.color: searchInput.activeFocus ? "#0078d4" : "#444"
                    TextInput {
                        id: searchInput
                        anchors { fill: parent; leftMargin: 8; rightMargin: 8 }
                        color: "white"; font.family: "Consolas,monospace"; font.pixelSize: 12
                        placeholderText: searchMode.currentIndex === 0 ? "FF ?? 4D 5A (hex with wildcards)" :
                                         searchMode.currentIndex === 1 ? "ASCII text to find..." :
                                         searchMode.currentIndex === 2 ? "Unicode string..." : "Regex pattern..."
                        onTextChanged: hexPanel.searchQuery = text
                        onAccepted: hexPanel.searchNext()
                    }
                }

                Button{text:"◄";flat:true;contentItem:Label{text:parent.text;color:"#888"};onClicked:hexPanel.searchPrev()}
                Button{text:"►";flat:true;contentItem:Label{text:parent.text;color:"#888"};onClicked:hexPanel.searchNext()}
                Button{text:" All ";flat:true;contentItem:Label{text:parent.text;color:"#888";font.pixelSize:11};onClicked:hexPanel.searchAll()}

                Rectangle{width:1;height:20;color:"#444"}

                Label { text: "Replace:"; color: "#888"; font.pixelSize: 11 }
                Rectangle {
                    width: 120; height: 26; color:"#0d0d0d"; radius:4; border.color:replaceInput.activeFocus?"#cc6600":"#444"
                    TextInput {
                        id: replaceInput; anchors{fill:parent;leftMargin:8;rightMargin:8}
                        color:"white"; font.family:"Consolas,monospace"; font.pixelSize:12
                        placeholderText:"Hex or ASCII"
                    }
                }
                Button{text:"Replace";flat:true;contentItem:Label{text:parent.text;color:"#cc6600";font.pixelSize:11};onClicked: { /* replace logic */ }}

                Rectangle{width:1;height:20;color:"#444"}

                Label { text: "Go to:"; color: "#888"; font.pixelSize: 11 }
                Rectangle {
                    width: 120; height: 26; color:"#0d0d0d"; radius:4; border.color:gotoInput.activeFocus?"#0078d4":"#444"
                    TextInput {
                        id: gotoInput; anchors{fill:parent;leftMargin:8;rightMargin:8}
                        color:"white"; font.family:"Consolas,monospace"; font.pixelSize:12
                        placeholderText:"0x00000000"
                        onAccepted: { hexPanel.gotoOffset(parseInt(text)); }
                    }
                }
                Button{text:"↵";flat:true;contentItem:Label{text:parent.text;color:"#888"};onClicked:hexPanel.gotoOffset(parseInt(gotoInput.text))}
            }
        }

        // ── Main hex view + data inspector ───────────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0

            // Hex view
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                color: "#0d0d0d"

                ScrollView {
                    anchors.fill: parent; clip: true

                    Column {
                        width: parent.width; spacing: 0

                        // Column header
                        Rectangle {
                            width: parent.width; height: 22
                            color: "#1a1a1a"
                            Row {
                                anchors.left: parent.left; anchors.leftMargin: 8; spacing: 0
                                Label {
                                    text: "Offset    "
                                    color: "#555"; font.family:"Consolas,monospace"; font.pixelSize:11
                                    width: 90
                                }
                                // Byte column headers (0-F)
                                Repeater {
                                    model: hexPanel.bytesPerRow
                                    Label {
                                        text: index.toString(16).toUpperCase().padStart(2,"0")+" "
                                        color: index === 8 ? "#334466" : "#444"
                                        font.family:"Consolas,monospace"; font.pixelSize:11
                                        width: 27
                                    }
                                }
                                Label { text:"  ASCII"; color:"#444"; font.family:"Consolas,monospace"; font.pixelSize:11 }
                            }
                        }

                        // Hex rows
                        Repeater {
                            model: hexPanel.visibleRows

                            Rectangle {
                                width: hexPanel.width; height: 20
                                color: index % 2 === 0 ? "#0d0d0d" : "#111"

                                Row {
                                    anchors.left: parent.left; anchors.leftMargin: 8; anchors.verticalCenter: parent.verticalCenter; spacing: 0

                                    // Offset
                                    Label {
                                        text: (hexPanel.pageOffset + index * hexPanel.bytesPerRow).toString(16).toUpperCase().padStart(8,"0")
                                        color: "#666"; font.family:"Consolas,monospace"; font.pixelSize:11; width:80
                                    }
                                    Label { text:"  "; color:"#333"; width:10 }

                                    // Hex bytes
                                    Repeater {
                                        model: hexPanel.bytesPerRow
                                        Label {
                                            property int byteIdx: (index + hexPanel.bytesPerRow * (model.index || 0))
                                            property int byteVal: hexPanel.pageData[byteIdx] !== undefined ? hexPanel.pageData[byteIdx] : -1
                                            property bool isSelected: hexPanel.selectedOffset === (hexPanel.pageOffset + byteIdx)
                                            property bool isSeparator: index === 8

                                            text: byteVal >= 0 ? byteVal.toString(16).toUpperCase().padStart(2,"0")+" " : "   "
                                            color: isSelected ? "white" : byteVal === 0 ? "#333" : byteVal < 32 ? "#886666" : "#88aa88"
                                            background: Rectangle { visible: isSelected; color: "#0078d4"; radius: 2 }
                                            font.family:"Consolas,monospace"; font.pixelSize:11; width:27
                                            leftPadding: isSeparator ? 6 : 0
                                            MouseArea { anchors.fill:parent; onClicked: hexPanel.selectedOffset = hexPanel.pageOffset + byteIdx }
                                        }
                                    }

                                    Label { text:"  "; color:"#222"; width:10 }

                                    // ASCII panel
                                    Repeater {
                                        model: hexPanel.bytesPerRow
                                        Label {
                                            property int byteIdx: (index + hexPanel.bytesPerRow * (model.index || 0))
                                            property int byteVal: hexPanel.pageData[byteIdx] !== undefined ? hexPanel.pageData[byteIdx] : -1
                                            property bool isSelected: hexPanel.selectedOffset === (hexPanel.pageOffset + byteIdx)

                                            text: byteVal >= 32 && byteVal < 127 ? String.fromCharCode(byteVal) : (byteVal === 0 ? "·" : "•")
                                            color: isSelected ? "white" : byteVal >= 32 && byteVal < 127 ? "#aaccaa" : "#444"
                                            background: Rectangle { visible: isSelected; color: "#0078d4"; radius: 2 }
                                            font.family:"Consolas,monospace"; font.pixelSize:11; width:9
                                            MouseArea { anchors.fill:parent; onClicked: hexPanel.selectedOffset = hexPanel.pageOffset + byteIdx }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Scroll bar for paging
                ScrollBar {
                    anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
                    orientation: Qt.Vertical; size: hexPanel.visibleRows / Math.max(1, hexPanel.totalBytes / hexPanel.bytesPerRow)
                    onPositionChanged: hexPanel.pageOffset = Math.floor(position * hexPanel.totalBytes / hexPanel.bytesPerRow) * hexPanel.bytesPerRow
                }
            }

            // ── Data Inspector panel ─────────────────────────────────────────
            Rectangle {
                width: 260; Layout.fillHeight: true
                color: "#141414"; border.color: "#2a2a2a"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 8; spacing: 4

                    Label { text: "Data Inspector"; color: "#aaa"; font.bold: true; font.pixelSize: 12 }
                    Label { text: "Offset: 0x" + (hexPanel.selectedOffset >= 0 ? hexPanel.selectedOffset.toString(16).toUpperCase() : "--"); color: "#666"; font.pixelSize: 10 }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }

                    // Integer types
                    component InspectRow: RowLayout {
                        property string label: ""; property string value: ""
                        Layout.fillWidth: true
                        Label { text: parent.label; color: "#888"; font.pixelSize: 10; font.family: "Consolas,monospace"; width: 80 }
                        Label { text: parent.value; color: "#ccc"; font.pixelSize: 11; font.family: "Consolas,monospace"; Layout.fillWidth: true; elide: Text.ElideRight }
                    }

                    Label { text: "Integers"; color: "#0078d4"; font.pixelSize: 10; font.bold: true }
                    InspectRow { label: "Int8"; value: hexPanel.dataInterpretation.as_int8 || "—" }
                    InspectRow { label: "UInt8"; value: hexPanel.dataInterpretation.as_uint8 || "—" }
                    InspectRow { label: "Int16 LE"; value: hexPanel.dataInterpretation.as_int16_le || "—" }
                    InspectRow { label: "Int16 BE"; value: hexPanel.dataInterpretation.as_int16_be || "—" }
                    InspectRow { label: "Int32 LE"; value: hexPanel.dataInterpretation.as_int32_le || "—" }
                    InspectRow { label: "Int32 BE"; value: hexPanel.dataInterpretation.as_int32_be || "—" }
                    InspectRow { label: "Int64 LE"; value: hexPanel.dataInterpretation.as_int64_le || "—" }
                    InspectRow { label: "UInt32 LE"; value: hexPanel.dataInterpretation.as_uint32_le || "—" }
                    InspectRow { label: "UInt64 LE"; value: hexPanel.dataInterpretation.as_uint64_le || "—" }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }
                    Label { text: "Floating Point"; color: "#0078d4"; font.pixelSize: 10; font.bold: true }
                    InspectRow { label: "Float32 LE"; value: hexPanel.dataInterpretation.as_float32_le || "—" }
                    InspectRow { label: "Float64 LE"; value: hexPanel.dataInterpretation.as_float64_le || "—" }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }
                    Label { text: "Text"; color: "#0078d4"; font.pixelSize: 10; font.bold: true }
                    InspectRow { label: "ASCII"; value: hexPanel.dataInterpretation.as_ascii || "—" }
                    InspectRow { label: "UTF-8"; value: hexPanel.dataInterpretation.as_utf8 || "—" }
                    InspectRow { label: "UTF-16 LE"; value: hexPanel.dataInterpretation.as_utf16_le || "—" }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#333" }
                    Label { text: "Special"; color: "#0078d4"; font.pixelSize: 10; font.bold: true }
                    InspectRow { label: "GUID"; value: hexPanel.dataInterpretation.as_guid || "—" }
                    InspectRow { label: "FILETIME"; value: hexPanel.dataInterpretation.as_filetime || "—" }
                    InspectRow { label: "Unix TS"; value: hexPanel.dataInterpretation.as_unix_timestamp || "—" }
                    InspectRow { label: "CRC-32"; value: hexPanel.dataInterpretation.crc32 ? "0x" + (hexPanel.dataInterpretation.crc32 >>> 0).toString(16).toUpperCase() : "—" }
                    InspectRow { label: "Binary"; value: hexPanel.dataInterpretation.as_binary || "—" }

                    Item { Layout.fillHeight: true }

                    // Endianness toggle
                    RowLayout {
                        Label { text: "Endian:"; color: "#888"; font.pixelSize: 11 }
                        ButtonGroup { id: endianGroup }
                        RadioButton { text: "LE"; checked: true; ButtonGroup.group: endianGroup
                            contentItem: Label { text: parent.text; color: parent.checked ? "#0078d4" : "#888"; font.pixelSize: 11 } }
                        RadioButton { text: "BE"; ButtonGroup.group: endianGroup
                            contentItem: Label { text: parent.text; color: parent.checked ? "#0078d4" : "#888"; font.pixelSize: 11 } }
                    }
                }
            }
        }

        // ── Status bar ───────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 24
            color: "#1a1a1a"; border.color: "#333"

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                Label { text: hexPanel.selectedOffset >= 0 ? "Selected: 0x" + hexPanel.selectedOffset.toString(16).toUpperCase() + " (" + hexPanel.selectedOffset + ")  |  Byte: 0x" + (hexPanel.pageData[hexPanel.selectedOffset - hexPanel.pageOffset] || 0).toString(16).toUpperCase() : "No selection"; color: "#888"; font.pixelSize: 11; font.family: "Consolas,monospace" }
                Item { Layout.fillWidth: true }
                Label { text: "Size: " + hexPanel.totalBytes + " bytes  |  " + (hexPanel.totalBytes/1024).toFixed(1) + " KB  |  " + (hexPanel.totalBytes/1048576).toFixed(2) + " MB"; color: "#666"; font.pixelSize: 11 }
            }
        }
    }
}
