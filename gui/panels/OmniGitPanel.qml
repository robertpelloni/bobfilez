import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniGitPanel.qml — Professional Version Control & Repo Manager.
/// Native libgit2 integration for committing, staging, and branching.
/// Shows visual commit graphs and diffs directly in the shell.

Rectangle {
    id: gitPanel
    color: "#050505"; radius: 8

    property string currentRepo: "C:/Users/hyper/workspace/bobfilez"
    property string currentBranch: "main"
    property int ahead: 2
    property int behind: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🐙 OmniGit VCM"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Pro Repository Manager"; color: "#888"; font.pixelSize: 14 }
            
            Rectangle {
                Layout.fillWidth: true; height: 32; radius: 6; color: "#1a1a1a"; border.color: "#333"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10
                    Label { text: "📂"; color: "#888" }
                    TextInput { text: gitPanel.currentRepo; color: "white"; Layout.fillWidth: true; font.pixelSize: 12; font.family: "Consolas" }
                }
            }

            Button {
                text: "Open / Clone..."
                background: Rectangle { color: "#252525"; radius: 4 }
                contentItem: Label { text: parent.text; color: "white" }
            }
        }

        // ── Toolbar ───────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 40; radius: 6; color: "#111"; border.color: "#222"
            RowLayout {
                anchors.fill: parent; anchors.margins: 5; spacing: 10
                
                // Branch info
                Rectangle {
                    width: 140; height: 30; radius: 4; color: "#1a3a5a"; border.color: "#0078d4"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 5
                        Label { text: "🌱 " + gitPanel.currentBranch; color: "white"; font.bold: true; font.pixelSize: 11; Layout.fillWidth: true }
                        Label { text: "↑" + gitPanel.ahead; color: "#4caf50"; font.pixelSize: 10; font.bold: true; visible: gitPanel.ahead > 0 }
                        Label { text: "↓" + gitPanel.behind; color: "#ffaa00"; font.pixelSize: 10; font.bold: true; visible: gitPanel.behind > 0 }
                    }
                }

                Rectangle { width: 1; height: 20; color: "#333" }

                Button { text: "↓ Pull"; flat: true; contentItem: Label { text: parent.text; color: "white" } }
                Button { text: "↑ Push"; background: Rectangle { color: "#0078d4"; radius: 4 }; contentItem: Label { text: parent.text; color: "white"; font.bold: true } }
                Button { text: "🌿 Branch"; flat: true; contentItem: Label { text: parent.text; color: "white" } }
                Button { text: "🔄 Stash"; flat: true; contentItem: Label { text: parent.text; color: "white" } }

                Item { Layout.fillWidth: true }
                Button { text: "⚙️ Git Settings"; flat: true; contentItem: Label { text: parent.text; color: "#666" } }
            }
        }

        // ── Main Workspace ────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

            // 1. Staging Area (Left)
            Rectangle {
                Layout.preferredWidth: 280; Layout.fillHeight: true
                color: "#161616"; border.color: "#333"; radius: 6

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 10
                    
                    Label { text: "STAGED CHANGES (0)"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                    Rectangle { Layout.fillWidth: true; height: 60; color: "#111"; radius: 4; border.color: "#222"
                        Label { anchors.centerIn: parent; text: "Drag files here to stage"; color: "#444"; font.pixelSize: 10 } }

                    Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                    Label { text: "UNSTAGED CHANGES (3)"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                    ListView {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 2; clip: true
                        model: [
                            {name: "omnigit_engine.cpp", path: "core/src/", status: "M", color: "#ffaa00"},
                            {name: "OmniGitPanel.qml", path: "gui/panels/", status: "A", color: "#4caf50"},
                            {name: "CMakeLists.txt", path: "", status: "M", color: "#ffaa00"}
                        ]
                        delegate: Rectangle {
                            width: parent.width; height: 30; color: itemHover.hovered ? "#252525" : "transparent"; radius: 4
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 5; spacing: 8
                                Rectangle { width: 14; height: 14; radius: 3; color: modelData.color
                                    Label { anchors.centerIn: parent; text: modelData.status; color: "black"; font.bold: true; font.pixelSize: 9 } }
                                Column { Layout.fillWidth: true; spacing: 0
                                    Label { text: modelData.name; color: "white"; font.pixelSize: 11; elide: Text.ElideRight; width: parent.width }
                                    Label { text: modelData.path; color: "#666"; font.pixelSize: 9; elide: Text.ElideRight; width: parent.width }
                                }
                                Button { text: "+"; flat: true; width: 20; contentItem: Label { text: parent.text; color: "#888"; anchors.centerIn: parent } }
                            }
                            HoverHandler { id: itemHover }
                        }
                    }

                    Rectangle { height: 1; Layout.fillWidth: true; color: "#333" }

                    // Commit Box
                    Rectangle {
                        Layout.fillWidth: true; height: 100; color: "#0a0a0a"; border.color: "#333"; radius: 4
                        ColumnLayout { anchors.fill: parent; anchors.margins: 8; spacing: 5
                            TextInput { Layout.fillWidth: true; color: "white"; font.pixelSize: 12; placeholderText: "Commit summary..." }
                            Rectangle { Layout.fillWidth: true; height: 1; color: "#222" }
                            TextArea { Layout.fillWidth: true; Layout.fillHeight: true; color: "#ccc"; font.pixelSize: 11; placeholderText: "Extended description..." }
                        }
                    }
                    Button { text: "Commit"; Layout.fillWidth: true; background: Rectangle { color: "#1a4a1a"; radius: 4; border.color: "#4caf50" }; contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true; horizontalAlignment: Text.AlignHCenter } }
                }
            }

            // 2. Commit Graph & Diff View (Right)
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 10

                // Graph View
                GroupBox {
                    label: Label { text: "Commit History"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                    Layout.fillWidth: true; Layout.fillHeight: true
                    background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                    padding: 0
                    
                    ListView {
                        anchors.fill: parent; clip: true; spacing: 0
                        model: [
                            {hash: "b34c932", msg: "feat: v4.1.0 - Self-Healing & Data Resurrection", author: "Antigravity", time: "2 hours ago", color: "#0078d4"},
                            {hash: "1f99a1c", msg: "feat: v4.0.0 - The Ultimate Distributed Shell", author: "Antigravity", time: "4 hours ago", color: "#ffaa00"},
                            {hash: "fcacaaa", msg: "feat: v3.9.0 - Media Asset Manager & Semantic Tagging", author: "Antigravity", time: "Yesterday", color: "#ffaa00"},
                            {hash: "ae1ebba", msg: "feat: v3.8.0 - Live Topology & Distributed Nexus", author: "Antigravity", time: "Yesterday", color: "#ffaa00"},
                            {hash: "1aeb597", msg: "feat: v3.7.0 - Forensic Auditing & Semantic Document Search", author: "Antigravity", time: "2 days ago", color: "#4caf50"}
                        ]
                        delegate: Rectangle {
                            width: parent.width; height: 36; color: index % 2 === 0 ? "#111" : "#1a1a1a"
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 10
                                // Graph Node Visual
                                Item { width: 30; height: 36
                                    Rectangle { width: 2; height: 36; color: modelData.color; anchors.centerIn: parent }
                                    Rectangle { width: 10; height: 10; radius: 5; color: modelData.color; border.color: "black"; border.width: 2; anchors.centerIn: parent }
                                }
                                Label { text: modelData.msg; color: "white"; font.pixelSize: 12; font.bold: index === 0; elide: Text.ElideRight; Layout.fillWidth: true }
                                Label { text: modelData.hash; color: "#666"; font.family: "Consolas"; font.pixelSize: 11 }
                                Label { text: modelData.author; color: "#888"; font.pixelSize: 11; width: 80 }
                                Label { text: modelData.time; color: "#555"; font.pixelSize: 10; width: 80; horizontalAlignment: Text.AlignRight }
                            }
                        }
                    }
                }

                // Diff Viewer
                GroupBox {
                    label: Label { text: "Diff: omnigit_engine.cpp"; color: "#aaa"; font.bold: true; font.pixelSize: 11 }
                    Layout.fillWidth: true; Layout.preferredHeight: 300
                    background: Rectangle { color: "#161616"; radius: 6; border.color: "#333" }
                    padding: 5
                    
                    Rectangle {
                        anchors.fill: parent; color: "#000"; radius: 4; border.color: "#222"
                        ScrollView {
                            anchors.fill: parent; clip: true
                            TextArea {
                                text: "@@ -0,0 +1,24 @@\n" +
                                      "+// New OmniGit engine implementation\n" +
                                      "+#include \"fo/core/omnigit_interface.hpp\"\n" +
                                      "+#include \"fo/core/registry.hpp\"\n" +
                                      "+\n" +
                                      "+namespace fo::core {\n" +
                                      "+\n" +
                                      "+class OmniGitEngineImpl : public IOmniGitEngine {\n" +
                                      "+\n" +
                                      "+};\n" +
                                      "+}\n"
                                color: "#d4d4d4"; font.family: "Consolas,monospace"; font.pixelSize: 12; readOnly: true
                                // Simulated syntax highlighting
                            }
                        }
                    }
                }
            }
        }
    }
}
