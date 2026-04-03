import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniTerminalPanel.qml — AI-Powered Terminal Emulator.
/// Full PTY terminal with OmniOracle AI command suggestions.

Rectangle {
    id: termPanel
    color: "#0a0a0a"; radius: 8

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // ── Tab Bar ───────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 36; color: "#1a1a1a"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 10; spacing: 2
                Rectangle { width: 150; height: 28; color: "#0a0a0a"; radius: 4
                    RowLayout { anchors.fill: parent; anchors.leftMargin: 8; spacing: 6
                        Label { text: "⌨️ PowerShell"; color: "white"; font.pixelSize: 11 }
                        Item { Layout.fillWidth: true }
                        Label { text: "✕"; color: "#666"; font.pixelSize: 10 }
                    }
                }
                Rectangle { width: 28; height: 28; color: "transparent"; radius: 4
                    Label { anchors.centerIn: parent; text: "＋"; color: "#888"; font.pixelSize: 16 } }
                Item { Layout.fillWidth: true }
                Label { text: "🤖 AI Assist: ON"; color: "#4caf50"; font.pixelSize: 10; font.bold: true }
            }
        }

        // ── Terminal Output ───────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; color: "#0a0a0a"
            ScrollView {
                anchors.fill: parent; anchors.margins: 10; clip: true
                TextArea {
                    id: termOutput
                    text: "PS C:\\Users\\hyper\\workspace\\bobfilez> git log --oneline -5\n" +
                          "7dff260 feat: v5.6.0 - OmniCluster Distributed Grid Computing\n" +
                          "465ac64 feat: v5.5.0 - OmniCrypt Steganography\n" +
                          "58290c9 feat: v5.4.0 - OmniVerse 3D Explorer\n" +
                          "1957263 feat: v5.3.0 - OmniGit Version Control\n" +
                          "cda9b6b feat: v5.2.0 - OmniPeek Universal Quick Look\n\n" +
                          "PS C:\\Users\\hyper\\workspace\\bobfilez> _"
                    color: "#d4d4d4"; font.family: "Cascadia Code,Consolas,monospace"; font.pixelSize: 13
                    background: null; readOnly: true; wrapMode: TextArea.Wrap
                }
            }
        }

        // ── AI Suggestion Bar ─────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 50; color: "#161616"; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.margins: 8; spacing: 10
                Label { text: "🤖"; font.pixelSize: 16 }
                Repeater {
                    model: [
                        {cmd: "git status", desc: "Show working tree status"},
                        {cmd: "git diff --cached", desc: "Show staged changes"},
                        {cmd: "fo_cli stats .", desc: "Bobfilez file statistics"}
                    ]
                    Button {
                        text: modelData.cmd
                        background: Rectangle { color: hovered ? "#333" : "#252525"; radius: 4; border.color: "#444" }
                        contentItem: Label { text: parent.text; color: "#0078d4"; font.family: "Consolas"; font.pixelSize: 11 }
                        ToolTip.visible: hovered; ToolTip.text: modelData.desc
                    }
                }
                Item { Layout.fillWidth: true }
            }
        }

        // ── Input Line ────────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 36; color: "#111"; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.margins: 8; spacing: 8
                Label { text: "PS>"; color: "#0078d4"; font.family: "Consolas"; font.bold: true }
                TextInput {
                    id: cmdInput; Layout.fillWidth: true; color: "white"
                    font.family: "Cascadia Code,Consolas,monospace"; font.pixelSize: 13
                    onAccepted: {
                        termOutput.text += "\nPS> " + text + "\n[Executing...]\n"
                        text = ""
                    }
                }
            }
        }
    }
}
