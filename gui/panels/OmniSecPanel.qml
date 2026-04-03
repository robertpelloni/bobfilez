import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// OmniSecPanel.qml — Advanced Cybersecurity and Reverse Engineering Suite.
/// Featuring radare2 for binary analysis, ssdeep for fuzzy hashing, and
/// hashcat for GPU-accelerated password recovery.

Rectangle {
    id: secPanel
    color: "#050505"; radius: 8

    property string targetFile: "C:/Downloads/suspicious_file.exe"
    property int activeTab: 0 // 0=Malware, 1=Disassembly, 2=Password Recovery
    property bool isCracking: false
    property bool isDisassembling: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "☠️ OmniSec Suite"; font.pixelSize: 22; font.bold: true; color: "#ff4444" }
            Label { text: "Forensics • Reversing • Cryptanalysis"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: 150; height: 24; radius: 12; color: "#3a1a1a"; border.color: "#ff4444"
                Label { anchors.centerIn: parent; text: "KERNEL MODE ACTIVE"; color: "#ff4444"; font.pixelSize: 10; font.bold: true }
            }
        }

        // ── File Selection & Tabs ─────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 50; radius: 8; color: "#161616"; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 15
                Label { text: "🎯 Target:"; color: "#ffaa00"; font.bold: true; font.pixelSize: 11 }
                TextInput { 
                    text: secPanel.targetFile; color: "white"; font.family: "Consolas"; font.pixelSize: 13
                    Layout.fillWidth: true; readOnly: true 
                }
                Button { text: "Browse..."; flat: true; contentItem: Label { text: parent.text; color: "#ffaa00" } }
            }
        }

        RowLayout {
            spacing: 5
            Repeater {
                model: [
                    {name: "🛡️ Malware Scan (ssdeep)", idx: 0},
                    {name: "🔬 Disassembly (radare2)", idx: 1},
                    {name: "🔓 Password Recovery (hashcat)", idx: 2}
                ]
                Button {
                    text: modelData.name
                    background: Rectangle { color: secPanel.activeTab === modelData.idx ? "#ff4444" : "#1a1a1a"; radius: 4 }
                    contentItem: Label { text: parent.text; color: secPanel.activeTab === modelData.idx ? "white" : "#aaa"; font.bold: true; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter }
                    onClicked: secPanel.activeTab = modelData.idx
                    Layout.preferredWidth: 200; height: 35
                }
            }
        }

        // ── Main Content Area ─────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#111"; radius: 6; border.color: "#333"
            clip: true

            // 1. Malware Scan (ssdeep)
            ColumnLayout {
                visible: secPanel.activeTab === 0
                anchors.fill: parent; anchors.margins: 20; spacing: 15
                
                Label { text: "Threat Analysis Report"; color: "#aaa"; font.bold: true; font.pixelSize: 14 }
                
                RowLayout {
                    spacing: 20; Layout.fillWidth: true
                    Rectangle { width: 120; height: 120; radius: 60; color: "transparent"; border.color: "#ff4444"; border.width: 5
                        Column { anchors.centerIn: parent; spacing: 2
                            Label { text: "85%"; color: "#ff4444"; font.pixelSize: 32; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                            Label { text: "Risk Score"; color: "#888"; font.pixelSize: 10; anchors.horizontalCenter: parent.horizontalCenter }
                        }
                    }
                    ColumnLayout {
                        spacing: 8; Layout.fillWidth: true
                        Label { text: "Fuzzy Hash (ssdeep):"; color: "#888"; font.pixelSize: 11; font.bold: true }
                        Label { text: "192:Lkq8Y+O8A+QJ2m3HnN5Z9pX4rT7vW1uS:Lk/8yQJ2pHnNZ9pX4rT7uS"; color: "#4caf50"; font.family: "Consolas"; font.pixelSize: 12 }
                        
                        Label { text: "Suspicious API Imports:"; color: "#ffaa00"; font.pixelSize: 11; font.bold: true; Layout.topMargin: 10 }
                        Flow {
                            spacing: 5; Layout.fillWidth: true
                            Repeater {
                                model: ["VirtualAlloc", "WriteProcessMemory", "CreateRemoteThread", "LoadLibraryA"]
                                Rectangle { width: lbl.implicitWidth + 20; height: 24; radius: 4; color: "#3a1a1a"; border.color: "#ff4444"
                                    Label { id: lbl; anchors.centerIn: parent; text: modelData; color: "#ffaa00"; font.family: "Consolas"; font.pixelSize: 11 } }
                            }
                        }
                    }
                }
                
                Item { Layout.fillHeight: true }
                RowLayout {
                    spacing: 15
                    Button { text: "🛡️ Quarantine to Vault"; background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }; contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true } }
                    Button { text: "🗑️ Secure Delete (Wipe)"; background: Rectangle { color: "#3a1a1a"; radius: 6; border.color: "#ff4444" }; contentItem: Label { text: parent.text; color: "#ff4444"; font.bold: true } }
                }
            }

            // 2. Disassembly (radare2)
            ColumnLayout {
                visible: secPanel.activeTab === 1
                anchors.fill: parent; anchors.margins: 10; spacing: 5

                RowLayout {
                    Label { text: "radare2 Interactive Disassembler"; color: "#aaa"; font.bold: true; font.pixelSize: 12 }
                    Item { Layout.fillWidth: true }
                    Button { text: "Decompile (Ghidra)"; flat: true; contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 11 } }
                }
                
                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; color: "#000"; radius: 4
                    ScrollView {
                        anchors.fill: parent; anchors.margins: 10
                        TextArea {
                            text: "[0x00401050]> pdf @ sym.main\n" +
                                  "┌ 64: sym.main (int argc, char **argv);\n" +
                                  "│           ; var int local_10h @ ebp-0x10\n" +
                                  "│           0x00401050      55             push ebp\n" +
                                  "│           0x00401051      89e5           mov ebp, esp\n" +
                                  "│           0x00401053      83ec10         sub esp, 0x10\n" +
                                  "│           0x00401056      e815000000     call sym.init\n" +
                                  "│           0x0040105b      31c0           xor eax, eax\n" +
                                  "└           0x0040105d      c9             leave\n" +
                                  "            0x0040105e      c3             ret\n\n" +
                                  "[0x00401050]> _"
                            color: "#00ff00"; font.family: "Consolas,monospace"; font.pixelSize: 12; readOnly: true
                        }
                    }
                }
            }

            // 3. Password Recovery (hashcat)
            ColumnLayout {
                visible: secPanel.activeTab === 2
                anchors.fill: parent; anchors.margins: 20; spacing: 15

                Label { text: "GPU-Accelerated Password Recovery (Hashcat)"; color: "#aaa"; font.bold: true; font.pixelSize: 14 }
                
                GridLayout {
                    columns: 2; columnSpacing: 15; rowSpacing: 10
                    Label { text: "Archive File:"; color: "#888" }
                    TextInput { text: "C:/Archives/locked_finances.zip"; color: "white"; font.family: "Consolas"; Layout.fillWidth: true; readOnly: true; background: Rectangle { color: "#252525"; radius: 4 } }
                    
                    Label { text: "Hash Type:"; color: "#888" }
                    ComboBox { model: ["Auto-Detect", "PKZIP", "RAR5", "7-Zip"]; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 } contentItem: Label { text: parent.currentText; color: "white"; leftPadding: 8 } }
                    
                    Label { text: "Dictionary:"; color: "#888" }
                    RowLayout {
                        Layout.fillWidth: true
                        TextInput { text: "rockyou.txt"; color: "white"; font.family: "Consolas"; Layout.fillWidth: true; background: Rectangle { color: "#252525"; radius: 4 } }
                        Button { text: "Browse"; flat: true }
                    }
                }

                Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                // Execution Status
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 8
                    
                    RowLayout {
                        Button {
                            text: secPanel.isCracking ? "⏹ Stop Crack" : "🚀 Start Crack"
                            background: Rectangle { color: secPanel.isCracking ? "#ff4444" : "#0078d4"; radius: 6 }
                            contentItem: Label { text: parent.text; color: "white"; font.bold: true }
                            onClicked: {
                                secPanel.isCracking = !secPanel.isCracking;
                            }
                        }
                        Item { Layout.fillWidth: true }
                        Label { visible: secPanel.isCracking; text: "Status: Running... ETA: 00:03:12  |  Speed: 1,200,000 H/s"; color: "#ffaa00"; font.pixelSize: 11; font.family: "Consolas" }
                    }

                    Rectangle {
                        Layout.fillWidth: true; height: 100; color: "#000"; radius: 4; border.color: "#333"
                        ScrollView {
                            anchors.fill: parent; anchors.margins: 10
                            TextArea {
                                text: secPanel.isCracking ? "Session..........: hashcat\nStatus...........: Running\nHash.Mode........: 13600 (WinZip)\nSpeed.#1.........:  1200.0 kH/s (42.06ms)\nRecovered........: 0/1 (0.00%)" :
                                      "\n\n\n\n\n[SUCCESS] Password Recovered: 'password123'"
                                color: secPanel.isCracking ? "#ffaa00" : "#4caf50"; font.family: "Consolas"; font.pixelSize: 11; readOnly: true
                            }
                        }
                    }
                }
            }
        }
    }
}
