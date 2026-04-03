import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// ─────────────────────────────────────────────────────────────────────────────
// FileOpsPanel.qml
// Comprehensive batch file operations panel:
//   Tab 1 — Copy/Move  (TeraCopy-style with verify, throttle, retry)
//   Tab 2 — Sync       (FreeFileSync-style mirror/two-way/update + versioning)
//   Tab 3 — Diff       (WinMerge-style folder & file diff with html view)
//   Tab 4 — Backup     (Areca/Duplicati-style full/incremental/differential + encryption)
//   Tab 5 — Archive    (7-Zip-style create/extract with all formats + password)
// ─────────────────────────────────────────────────────────────────────────────
Rectangle {
    id: fileOpsPanel
    color: "#161616"; radius: 8

    property int activeTab: 0
    property bool isRunning: false
    property int progressDone: 0; property int progressTotal: 0
    property double progressBytes: 0; property double totalBytes: 0

    signal startOperation()
    signal cancelOperation()

    // Re-usable styled TextField
    component StyledField: TextField {
        color: "white"; font.pixelSize: 12
        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
    }
    component StyledCombo: ComboBox {
        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
        contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 8; font.pixelSize: 12 }
    }
    component GroupLabel: Label { color: "#aaa"; font.bold: true }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 12; spacing: 10

        // ── Tab bar ─────────────────────────────────────────────────────────
        RowLayout {
            spacing: 4
            Label { text: "📦 File Operations"; font.pixelSize: 18; font.bold: true; color: "white" }
            Item { width: 16 }
            Repeater {
                model: ["📋 Copy/Move", "🔁 Sync", "⚖️ Diff", "💾 Backup", "🗜 Archive"]
                Rectangle {
                    width: tabLabel.implicitWidth + 20; height: 32; radius: 4
                    color: fileOpsPanel.activeTab === index ? "#0078d4" : "#252525"
                    border.color: fileOpsPanel.activeTab === index ? "#0078d4" : "#444"
                    Label { id: tabLabel; anchors.centerIn: parent; text: modelData; color: "white"; font.pixelSize: 12 }
                    MouseArea { anchors.fill: parent; onClicked: fileOpsPanel.activeTab = index }
                }
            }
        }

        // ────────────────────────────────────────────────────────────────────
        // TAB 0 — COPY / MOVE
        // ────────────────────────────────────────────────────────────────────
        ColumnLayout {
            visible: fileOpsPanel.activeTab === 0
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            RowLayout {
                spacing: 10; Layout.fillWidth: true

                GroupBox {
                    label: GroupLabel { text: "Source Files / Folders" }
                    Layout.fillWidth: true; Layout.fillHeight: true
                    background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }; padding: 8

                    ColumnLayout { anchors.fill: parent; spacing: 6
                        RowLayout {
                            Button { text: "＋ Files"
                                background: Rectangle { color: "#0078d4"; radius: 4 }
                                contentItem: Label { text: parent.text; color: "white" } }
                            Button { text: "📁 Folder"
                                background: Rectangle { color: "#252525"; radius: 4; border.color: "#444" }
                                contentItem: Label { text: parent.text; color: "white" } }
                            Button { text: "🗑 Clear"; flat: true; contentItem: Label { text: parent.text; color: "#ff6b6b" } }
                        }
                        ListView { Layout.fillWidth: true; height: 120; clip: true
                            model: 0; ScrollBar.vertical: ScrollBar {}
                            delegate: Rectangle { width: parent.width; height: 26; color: "#1a1a1a"
                                Label { text: modelData; color: "#ccc"; leftPadding: 8; font.pixelSize: 11 } } }
                    }
                }

                GroupBox {
                    label: GroupLabel { text: "Destination" }
                    Layout.preferredWidth: 300; Layout.fillHeight: true
                    background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }; padding: 8

                    ColumnLayout { anchors.fill: parent; spacing: 8
                        StyledField { Layout.fillWidth: true; placeholderText: "Destination folder..." }
                        Button { text: "📂 Browse"; Layout.fillWidth: true
                            background: Rectangle { color: "#252525"; radius: 4; border.color: "#555" }
                            contentItem: Label { text: parent.text; color: "white" } }

                        Label { text: "Operation:"; color: "#888"; font.pixelSize: 11 }
                        ButtonGroup { id: opGroup }
                        RadioButton { text: "Copy"; ButtonGroup.group: opGroup; checked: true
                            contentItem: Label { text: parent.text; color: "#ccc" } }
                        RadioButton { text: "Move"; ButtonGroup.group: opGroup
                            contentItem: Label { text: parent.text; color: "#ccc" } }
                        RadioButton { text: "Hardlink (NTFS)"; ButtonGroup.group: opGroup
                            contentItem: Label { text: parent.text; color: "#ccc" } }
                        RadioButton { text: "Symlink"; ButtonGroup.group: opGroup
                            contentItem: Label { text: parent.text; color: "#ccc" } }
                    }
                }
            }

            GroupBox {
                label: GroupLabel { text: "Options" }
                Layout.fillWidth: true
                background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }; padding: 10

                GridLayout { anchors.fill: parent; columns: 6; columnSpacing: 16; rowSpacing: 8

                    Label { text: "Collision:"; color: "#888"; font.pixelSize: 11 }
                    StyledCombo { model: ["Skip existing","Overwrite","Rename auto","Overwrite if newer","Overwrite if smaller","Ask per-file","Abort"]; Layout.fillWidth: true }

                    Label { text: "Threads:"; color: "#888"; font.pixelSize: 11 }
                    RowLayout { Slider { from:1;to:16;value:4;stepSize:1; Layout.fillWidth: true }; Label { text:"4"; color:"#ccc"; width:20 } }

                    Label { text: "Throttle:"; color: "#888"; font.pixelSize: 11 }
                    StyledCombo { model: ["No limit","100 MB/s","50 MB/s","10 MB/s","1 MB/s"]; Layout.fillWidth: true }

                    Label { text: "Verify:"; color: "#888"; font.pixelSize: 11 }
                    StyledCombo { model: ["None (fast)","xxHash (fast)","MD5","SHA-256"]; Layout.fillWidth: true }

                    Label { text: "Retry:"; color: "#888"; font.pixelSize: 11 }
                    RowLayout {
                        SpinBox { from:0;to:10;value:3; implicitWidth:70
                            background: Rectangle { color:"#2d2d2d";radius:4;border.color:"#555" }
                            contentItem: TextInput { text:parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter } }
                        Label { text: "× delay"; color:"#888"; font.pixelSize:11 }
                        SpinBox { from:0;to:30000;value:1000; implicitWidth:80
                            background: Rectangle { color:"#2d2d2d";radius:4;border.color:"#555" }
                            contentItem: TextInput { text:parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter } }
                        Label { text:"ms"; color:"#888"; font.pixelSize:11 }
                    }

                    CheckBox { contentItem: Label{text:"Preserve timestamps";color:"#ccc";leftPadding:4;font.pixelSize:11} checked:true }
                    CheckBox { contentItem: Label{text:"Preserve attributes";color:"#ccc";leftPadding:4;font.pixelSize:11} checked:true }
                    CheckBox { contentItem: Label{text:"Skip hidden files";color:"#ccc";leftPadding:4;font.pixelSize:11} }
                    CheckBox { contentItem: Label{text:"Skip system files";color:"#ccc";leftPadding:4;font.pixelSize:11} }
                    CheckBox { contentItem: Label{text:"Flat copy (no subdir)";color:"#ccc";leftPadding:4;font.pixelSize:11} }
                    CheckBox { contentItem: Label{text:"Delete orphans in dest";color:"#ccc";leftPadding:4;font.pixelSize:11} }

                    Label { text:"Include ext:"; color:"#888"; font.pixelSize:11 }
                    StyledField { Layout.fillWidth:true; placeholderText:"*.jpg *.mp4 (blank=all)" }
                    Label { text:"Exclude ext:"; color:"#888"; font.pixelSize:11 }
                    StyledField { Layout.fillWidth:true; placeholderText:"*.tmp *.bak" }
                    Label { text:"Exclude dirs:"; color:"#888"; font.pixelSize:11 }
                    StyledField { Layout.fillWidth:true; placeholderText:"node_modules .git" }
                }
            }

            RowLayout { spacing:10
                Button { text:"🔍 Preview (Dry Run)"; background:Rectangle{color:"#2d2d2d";radius:6;border.color:"#0078d4"}; contentItem:Label{text:parent.text;color:"#0078d4";font.bold:true} }
                Button { text:fileOpsPanel.isRunning?"⏹ Cancel":"▶ Start"; onClicked:fileOpsPanel.isRunning?fileOpsPanel.cancelOperation():fileOpsPanel.startOperation()
                    background:Rectangle{color:fileOpsPanel.isRunning?"#cc3333":"#0078d4";radius:6}; contentItem:Label{text:parent.text;color:"white";font.bold:true} }
                Item { Layout.fillWidth:true }
            }
        }

        // ────────────────────────────────────────────────────────────────────
        // TAB 1 — SYNC (FreeFileSync-style)
        // ────────────────────────────────────────────────────────────────────
        ColumnLayout {
            visible: fileOpsPanel.activeTab === 1
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            RowLayout { spacing: 10; Layout.fillWidth: true
                GroupBox { label: GroupLabel{text:"Left Folder"}; Layout.fillWidth:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:8
                    RowLayout { anchors.fill:parent; StyledField{Layout.fillWidth:true;placeholderText:"Left folder..."} ; Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}} } }

                Label { text:"⇄"; font.pixelSize:24; color:"#0078d4"; font.bold:true }

                GroupBox { label: GroupLabel{text:"Right Folder"}; Layout.fillWidth:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:8
                    RowLayout { anchors.fill:parent; StyledField{Layout.fillWidth:true;placeholderText:"Right folder..."} ; Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}} } }
            }

            GroupBox { label: GroupLabel{text:"Sync Mode"}; Layout.fillWidth:true
                background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:10

                ColumnLayout { anchors.fill:parent; spacing:10
                    RowLayout { spacing:20
                        ButtonGroup { id:syncModeGroup }
                        Repeater {
                            model: [
                                {name:"Mirror",icon:"→",desc:"Make right identical to left\n(deletes extras in right)"},
                                {name:"Update",icon:"→+",desc:"Copy newer/missing left→right\n(never deletes)"},
                                {name:"Two-Way",icon:"⇄",desc:"Bidirectional sync\n(newest version wins)"},
                                {name:"Custom",icon:"⚙",desc:"Configure per-category\nactions manually"}
                            ]
                            Rectangle {
                                width:160; height:80; radius:8
                                color:syncModeBtn.checked?"#1a3a5a":"#252525"
                                border.color:syncModeBtn.checked?"#0078d4":"#444"
                                RadioButton { id:syncModeBtn; checked:index===0; ButtonGroup.group:syncModeGroup; visible:false }
                                Column { anchors.centerIn:parent; spacing:4
                                    Label{text:modelData.icon;font.pixelSize:20;color:syncModeBtn.checked?"#0078d4":"#888";anchors.horizontalCenter:parent.horizontalCenter}
                                    Label{text:modelData.name;color:"white";font.bold:true;anchors.horizontalCenter:parent.horizontalCenter}
                                    Label{text:modelData.desc;color:"#888";font.pixelSize:9;horizontalAlignment:Text.AlignHCenter} }
                                MouseArea{anchors.fill:parent;onClicked:syncModeBtn.checked=true}
                            }
                        }
                    }

                    GridLayout { columns:4; columnSpacing:16; rowSpacing:8

                        Label{text:"Compare by:";color:"#888";font.pixelSize:11}
                        StyledCombo{model:["File size + date (fast)","File content (accurate)","File size only"];Layout.fillWidth:true}

                        Label{text:"Time tolerance:";color:"#888";font.pixelSize:11}
                        RowLayout { SpinBox{from:0;to:10;value:2;implicitWidth:70;background:Rectangle{color:"#2d2d2d";radius:4;border.color:"#555"};contentItem:TextInput{text:parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter}}; Label{text:"sec (FAT32 fix)";color:"#888";font.pixelSize:11} }

                        CheckBox{contentItem:Label{text:"Versioning (keep old files)";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        StyledCombo{model:["Recycle bin","Timestamp folder","Delete permanently"];Layout.fillWidth:true}

                        Label{text:"Keep versions:";color:"#888";font.pixelSize:11}
                        SpinBox{from:1;to:100;value:10;implicitWidth:70;background:Rectangle{color:"#2d2d2d";radius:4;border.color:"#555"};contentItem:TextInput{text:parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter}}

                        CheckBox{contentItem:Label{text:"Detect moved/renamed files";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                        CheckBox{contentItem:Label{text:"Use sync database (FFS-style)";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                        CheckBox{contentItem:Label{text:"Real-time watch (auto-sync)";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        CheckBox{contentItem:Label{text:"Follow symlinks";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    }
                }
            }

            RowLayout { spacing:10
                Button{text:"🔍 Compare Folders";background:Rectangle{color:"#2d2d2d";radius:6;border.color:"#0078d4"};contentItem:Label{text:parent.text;color:"#0078d4";font.bold:true}}
                Button{text:fileOpsPanel.isRunning?"⏹ Cancel":"▶ Synchronize";onClicked:fileOpsPanel.isRunning?fileOpsPanel.cancelOperation():fileOpsPanel.startOperation()
                    background:Rectangle{color:fileOpsPanel.isRunning?"#cc3333":"#0078d4";radius:6};contentItem:Label{text:parent.text;color:"white";font.bold:true}}
                Item{Layout.fillWidth:true}
            }
        }

        // ────────────────────────────────────────────────────────────────────
        // TAB 2 — DIFF
        // ────────────────────────────────────────────────────────────────────
        ColumnLayout {
            visible: fileOpsPanel.activeTab === 2
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            RowLayout { spacing:10; Layout.fillWidth:true
                GroupBox{label:GroupLabel{text:"Left File / Folder"};Layout.fillWidth:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:8
                    RowLayout{anchors.fill:parent;StyledField{Layout.fillWidth:true;placeholderText:"Left side..."};Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}}}}
                Label{text:"vs";color:"#888";font.pixelSize:18}
                GroupBox{label:GroupLabel{text:"Right File / Folder"};Layout.fillWidth:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:8
                    RowLayout{anchors.fill:parent;StyledField{Layout.fillWidth:true;placeholderText:"Right side..."};Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}}}}
            }

            GroupBox{label:GroupLabel{text:"Diff Options"};Layout.fillWidth:true
                background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:10
                GridLayout{anchors.fill:parent;columns:6;columnSpacing:16;rowSpacing:8
                    Label{text:"Algorithm:";color:"#888";font.pixelSize:11}
                    StyledCombo{model:["Histogram (best for code)","Myers (classic git)","Patience (moved blocks)","Minimal"];Layout.fillWidth:true}
                    Label{text:"View:";color:"#888";font.pixelSize:11}
                    StyledCombo{model:["Side-by-side","Unified","HTML report"];Layout.fillWidth:true}
                    Label{text:"Context:";color:"#888";font.pixelSize:11}
                    SpinBox{from:0;to:50;value:3;implicitWidth:70;background:Rectangle{color:"#2d2d2d";radius:4;border.color:"#555"};contentItem:TextInput{text:parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter}}
                    CheckBox{contentItem:Label{text:"Ignore whitespace";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    CheckBox{contentItem:Label{text:"Ignore blank lines";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    CheckBox{contentItem:Label{text:"Ignore case";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    CheckBox{contentItem:Label{text:"Show identical files";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    CheckBox{contentItem:Label{text:"Binary (hex) compare";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    CheckBox{contentItem:Label{text:"Recursive folders";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                }}

            RowLayout{spacing:10
                Button{text:"⚖️ Run Diff";background:Rectangle{color:"#0078d4";radius:6};contentItem:Label{text:parent.text;color:"white";font.bold:true};onClicked:fileOpsPanel.startOperation()}
                Button{text:"💾 Export HTML";flat:true;contentItem:Label{text:parent.text;color:"#888"}}
                Button{text:"📋 Export Unified Diff";flat:true;contentItem:Label{text:parent.text;color:"#888"}}
                Item{Layout.fillWidth:true}}

            // Diff result view
            Rectangle { Layout.fillWidth:true; Layout.fillHeight:true; color:"#111"; radius:6; border.color:"#333"
                Label{anchors.centerIn:parent;text:"Diff results will appear here after running a comparison.";color:"#444"} }
        }

        // ────────────────────────────────────────────────────────────────────
        // TAB 3 — BACKUP
        // ────────────────────────────────────────────────────────────────────
        ColumnLayout {
            visible: fileOpsPanel.activeTab === 3
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            RowLayout{spacing:10;Layout.fillWidth:true
                GroupBox{label:GroupLabel{text:"Source"};Layout.fillWidth:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:8
                    ColumnLayout{anchors.fill:parent;spacing:6
                        RowLayout{StyledField{Layout.fillWidth:true;placeholderText:"Source folder..."};Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}}}
                        RowLayout{CheckBox{contentItem:Label{text:"Include subfolders";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                            CheckBox{contentItem:Label{text:"Skip hidden";color:"#ccc";leftPadding:4;font.pixelSize:11}}}
                        StyledField{Layout.fillWidth:true;placeholderText:"Exclude patterns: *.tmp *.bak node_modules/"}}}

                GroupBox{label:GroupLabel{text:"Destination"};Layout.fillWidth:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:8
                    ColumnLayout{anchors.fill:parent;spacing:6
                        RowLayout{StyledField{Layout.fillWidth:true;placeholderText:"Backup destination folder..."};Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}}}
                        Label{text:"Backup history:";color:"#888";font.pixelSize:11}
                        ListView{Layout.fillWidth:true;height:80;clip:true;model:0;ScrollBar.vertical:ScrollBar{}
                            delegate:Rectangle{width:parent.width;height:24;color:"#1a1a1a";RowLayout{anchors.fill:parent;anchors.leftMargin:8;Label{text:"No backups yet";color:"#555";font.pixelSize:11}}}}}
                }
            }

            GroupBox{label:GroupLabel{text:"Backup Configuration"};Layout.fillWidth:true
                background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:10
                GridLayout{anchors.fill:parent;columns:6;columnSpacing:16;rowSpacing:8
                    Label{text:"Type:";color:"#888";font.pixelSize:11}
                    StyledCombo{model:["Full backup","Incremental","Differential","Mirror+Versioning"];Layout.fillWidth:true}
                    Label{text:"Compress:";color:"#888";font.pixelSize:11}
                    StyledCombo{model:["Zstd (fast+ratio)","LZ4 (fastest)","Brotli (small)","ZIP","7z (ultra)","None (fast)"];Layout.fillWidth:true}
                    Label{text:"Level:";color:"#888";font.pixelSize:11}
                    RowLayout{Slider{id:cmpLevel;from:1;to:22;value:6;Layout.fillWidth:true};Label{text:Math.round(cmpLevel.value);color:"#ccc";width:24}}

                    CheckBox{contentItem:Label{text:"Encrypt backup";color:"#ccc";leftPadding:4;font.pixelSize:11}id:encryptCheck}
                    StyledField{Layout.fillWidth:true;echoMode:TextInput.Password;placeholderText:"Password...";enabled:encryptCheck.checked}
                    Label{text:"Cipher:";color:"#888";font.pixelSize:11}
                    StyledCombo{model:["AES-256-GCM","ChaCha20-Poly1305"];enabled:encryptCheck.checked;Layout.fillWidth:true}

                    CheckBox{contentItem:Label{text:"Split volumes";color:"#ccc";leftPadding:4;font.pixelSize:11}id:splitCheck}
                    RowLayout{enabled:splitCheck.checked
                        StyledField{width:80;placeholderText:"Size"};StyledCombo{model:["MB","GB","TB"];implicitWidth:70}}

                    Label{text:"Keep versions:";color:"#888";font.pixelSize:11}
                    SpinBox{from:1;to:999;value:10;implicitWidth:80;background:Rectangle{color:"#2d2d2d";radius:4;border.color:"#555"};contentItem:TextInput{text:parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter}}

                    CheckBox{contentItem:Label{text:"Verify after backup";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                    CheckBox{contentItem:Label{text:"Delta compression (rsync-style)";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                    CheckBox{contentItem:Label{text:"Use rsync algorithm";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                }}

            RowLayout{spacing:10
                Button{text:"▶ Run Backup";onClicked:fileOpsPanel.startOperation();background:Rectangle{color:"#0078d4";radius:6};contentItem:Label{text:parent.text;color:"white";font.bold:true}}
                Button{text:"📂 Restore...";flat:true;contentItem:Label{text:parent.text;color:"#888"}}
                Button{text:"🗑 Delete Version";flat:true;contentItem:Label{text:parent.text;color:"#ff6b6b"}}
                Item{Layout.fillWidth:true}
            }
        }

        // ────────────────────────────────────────────────────────────────────
        // TAB 4 — ARCHIVE (7-Zip-style)
        // ────────────────────────────────────────────────────────────────────
        ColumnLayout {
            visible: fileOpsPanel.activeTab === 4
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            RowLayout { spacing:10; Layout.fillWidth:true
                // Left: Create archive
                GroupBox{label:GroupLabel{text:"Create Archive"};Layout.fillWidth:true;Layout.fillHeight:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:10
                    ColumnLayout{anchors.fill:parent;spacing:8
                        RowLayout{Label{text:"Files/Folders:";color:"#888";font.pixelSize:11};Button{text:"＋ Add";background:Rectangle{color:"#0078d4";radius:4};contentItem:Label{text:parent.text;color:"white"}}}
                        ListView{Layout.fillWidth:true;height:100;clip:true;model:0;ScrollBar.vertical:ScrollBar{};border:Rectangle{color:"#333"}}
                        GridLayout{columns:2;columnSpacing:12;rowSpacing:6
                            Label{text:"Archive file:";color:"#888";font.pixelSize:11}
                            StyledField{Layout.fillWidth:true;placeholderText:"output.7z"}
                            Label{text:"Format:";color:"#888";font.pixelSize:11}
                            StyledCombo{model:["7-Zip (.7z)","ZIP (.zip)","TAR+Zstd (.tar.zst)","TAR+LZ4 (.tar.lz4)","TAR+Gzip (.tar.gz)","TAR+BZ2 (.tar.bz2)","TAR+XZ (.tar.xz)","TAR (no compress)"];Layout.fillWidth:true}
                            Label{text:"Level:";color:"#888";font.pixelSize:11}
                            StyledCombo{model:["Store (0)","Fastest (1)","Fast (3)","Normal (5)","Maximum (7)","Ultra (9)"];currentIndex:4;Layout.fillWidth:true}
                            Label{text:"Method:";color:"#888";font.pixelSize:11}
                            StyledCombo{model:["LZMA2 (default)","Deflate","BZip2","Zstd","LZ4","Copy"];Layout.fillWidth:true}
                            Label{text:"Threads:";color:"#888";font.pixelSize:11}
                            SpinBox{from:1;to:64;value:0;implicitWidth:70;background:Rectangle{color:"#2d2d2d";radius:4;border.color:"#555"};contentItem:TextInput{text:parent.value===0?"auto":parent.value;color:"white";horizontalAlignment:Qt.AlignHCenter}}
                        }
                        CheckBox{id:archEncrypt;contentItem:Label{text:"Encrypt (AES-256)";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        StyledField{echoMode:TextInput.Password;placeholderText:"Password...";enabled:archEncrypt.checked;Layout.fillWidth:true}
                        CheckBox{contentItem:Label{text:"Encrypt filenames (7z only)";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        CheckBox{contentItem:Label{text:"Solid mode (better ratio)";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                        CheckBox{contentItem:Label{text:"Create self-extracting EXE";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        CheckBox{contentItem:Label{text:"Split volumes";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        CheckBox{contentItem:Label{text:"Delete files after archive";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        Button{text:"🗜 Create Archive";Layout.fillWidth:true;onClicked:fileOpsPanel.startOperation()
                            background:Rectangle{color:"#0078d4";radius:6};contentItem:Label{text:parent.text;color:"white";font.bold:true;horizontalAlignment:Text.AlignHCenter}}
                    }
                }

                // Right: Extract
                GroupBox{label:GroupLabel{text:"Extract Archive"};Layout.fillWidth:true;Layout.fillHeight:true
                    background:Rectangle{color:"#1e1e1e";radius:6;border.color:"#333"};padding:10
                    ColumnLayout{anchors.fill:parent;spacing:8
                        Label{text:"Supported: zip, 7z, tar.*, gz, bz2, xz, zst, lz4, rar(read), iso, cab, wim, ar, deb, rpm, ...";color:"#666";font.pixelSize:10;wrapMode:Text.Wrap;Layout.fillWidth:true}
                        RowLayout{StyledField{Layout.fillWidth:true;placeholderText:"Archive file to extract..."};Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}}}
                        RowLayout{Label{text:"Destination:";color:"#888";font.pixelSize:11};StyledField{Layout.fillWidth:true;placeholderText:"Extract to folder..."};Button{text:"📂";flat:true;contentItem:Label{text:parent.text;color:"white"}}}
                        ButtonGroup{id:extractDest}
                        RadioButton{text:"Extract to: same folder";ButtonGroup.group:extractDest;checked:true;contentItem:Label{text:parent.text;color:"#ccc"}}
                        RadioButton{text:"Extract to: subfolder with archive name";ButtonGroup.group:extractDest;contentItem:Label{text:parent.text;color:"#ccc"}}
                        RadioButton{text:"Extract to: custom folder above";ButtonGroup.group:extractDest;contentItem:Label{text:parent.text;color:"#ccc"}}
                        StyledField{Layout.fillWidth:true;echoMode:TextInput.Password;placeholderText:"Password (if encrypted)..."}
                        CheckBox{contentItem:Label{text:"Overwrite existing files";color:"#ccc";leftPadding:4;font.pixelSize:11}}
                        CheckBox{contentItem:Label{text:"Keep directory structure";color:"#ccc";leftPadding:4;font.pixelSize:11}checked:true}
                        CheckBox{contentItem:Label{text:"Verify integrity after extract";color:"#ccc";leftPadding:4;font.pixelSize:11}}

                        // Archive contents preview
                        Label{text:"Archive Contents:";color:"#aaa";font.pixelSize:11;font.bold:true}
                        Rectangle{Layout.fillWidth:true;Layout.fillHeight:true;color:"#111";radius:4;border.color:"#333"
                            Label{anchors.centerIn:parent;text:"Open an archive to browse contents";color:"#444"}}

                        RowLayout{
                            Button{text:"🔍 Browse Contents";flat:true;contentItem:Label{text:parent.text;color:"#888"}}
                            Button{text:"✅ Test Archive";flat:true;contentItem:Label{text:parent.text;color:"#888"}}
                            Item{Layout.fillWidth:true}
                            Button{text:"📤 Extract";Layout.preferredWidth:120;onClicked:fileOpsPanel.startOperation()
                                background:Rectangle{color:"#0078d4";radius:6};contentItem:Label{text:parent.text;color:"white";font.bold:true;horizontalAlignment:Text.AlignHCenter}}
                        }
                    }
                }
            }
        }

        // ── Progress (shown during operation) ───────────────────────────────
        GroupBox {
            visible: fileOpsPanel.isRunning || fileOpsPanel.progressDone > 0
            label: GroupLabel{text:"Progress"}
            Layout.fillWidth: true
            background: Rectangle { color:"#1e1e1e"; radius:6; border.color:"#333" }; padding:8

            ColumnLayout { anchors.fill:parent; spacing:6
                RowLayout {
                    Label{text:"Files:";color:"#888";font.pixelSize:11}
                    ProgressBar { Layout.fillWidth:true; value:fileOpsPanel.progressTotal>0?fileOpsPanel.progressDone/fileOpsPanel.progressTotal:0
                        background:Rectangle{color:"#2d2d2d";radius:3;implicitHeight:10}
                        contentItem:Rectangle{color:"#0078d4";radius:3;width:parent.width*parent.value} }
                    Label{text:fileOpsPanel.progressDone+" / "+fileOpsPanel.progressTotal;color:"#888";font.pixelSize:11}
                }
                RowLayout {
                    Label{text:"Data:";color:"#888";font.pixelSize:11}
                    ProgressBar { Layout.fillWidth:true; value:fileOpsPanel.totalBytes>0?fileOpsPanel.progressBytes/fileOpsPanel.totalBytes:0
                        background:Rectangle{color:"#2d2d2d";radius:3;implicitHeight:10}
                        contentItem:Rectangle{color:"#4caf50";radius:3;width:parent.width*parent.value} }
                    Label{text:(fileOpsPanel.progressBytes/1048576).toFixed(1)+" MB / "+(fileOpsPanel.totalBytes/1048576).toFixed(1)+" MB";color:"#888";font.pixelSize:11}
                }
            }
        }
    }
}
