import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OraclePanel.qml — Local LLM Copilot & RAG Assistant for OmniShell.
/// Chat with your filesystem to organize files, summarize documents,
/// and execute complex OmniFlow automations.

Rectangle {
    id: oraclePanel
    color: "#0a0a0a"; radius: 8

    property bool isTyping: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 15
            Label { text: "🤖 OmniOracle"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Local RAG Assistant (Phi-3 3.8B)"; color: "#888"; font.pixelSize: 14 }
            
            Rectangle { width: 80; height: 24; radius: 12; color: "#1a3a1a"
                Label { anchors.centerIn: parent; text: "1.2 Tokens/s"; color: "#4caf50"; font.pixelSize: 10; font.bold: true } }
            
            Item { Layout.fillWidth: true }
            
            Button { text: "⚙️ Settings"; flat: true; contentItem: Label { text: parent.text; color: "#666" } }
        }

        // ── Chat Timeline ─────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; color: "#111"; radius: 10; border.color: "#333"
            clip: true

            ListView {
                anchors.fill: parent; anchors.margins: 15; spacing: 20
                ScrollBar.vertical: ScrollBar {}
                model: 2

                delegate: Item {
                    width: parent.width
                    height: bubbleCol.implicitHeight

                    ColumnLayout {
                        id: bubbleCol
                        width: parent.width; spacing: 10

                        // ── User Message ──
                        Rectangle {
                            Layout.alignment: Qt.AlignRight; Layout.maximumWidth: parent.width * 0.7; height: userText.implicitHeight + 20
                            color: "#0078d4"; radius: 15
                            Label {
                                id: userText; anchors.centerIn: parent; width: parent.width - 20; color: "white"; font.pixelSize: 13; wrapMode: Text.WordWrap
                                text: index === 0 ? "Summarize the Q3 report." : "Organize all my invoices from last year and encrypt them."
                            }
                        }

                        // ── Oracle Message ──
                        RowLayout {
                            Layout.alignment: Qt.AlignLeft; Layout.maximumWidth: parent.width * 0.85; spacing: 15
                            
                            Rectangle { width: 40; height: 40; radius: 20; color: "#252525"; border.color: "#0078d4"
                                Label { anchors.centerIn: parent; text: "🤖"; font.pixelSize: 20 }
                            }
                            
                            Rectangle {
                                Layout.fillWidth: true; Layout.preferredHeight: oracleContent.implicitHeight + 20
                                color: "#1e1e1e"; radius: 15; border.color: "#333"

                                ColumnLayout {
                                    id: oracleContent; anchors.fill: parent; anchors.margins: 15; spacing: 10
                                    
                                    Label {
                                        Layout.fillWidth: true; color: "#ccc"; font.pixelSize: 13; wrapMode: Text.WordWrap
                                        text: index === 0 ? "This is a 24-page PDF document that outlines the Q3 financial budget. The primary expense is Marketing ($45k), followed by R&D ($30k). The conclusion recommends a 10% cut in software licensing." : 
                                                            "I found 142 unencrypted financial documents scattered across your Downloads and Documents folders. I recommend running the 'Auto-Vault Financials' OmniFlow to secure them."
                                    }

                                    // ── Citations ──
                                    Flow {
                                        Layout.fillWidth: true; spacing: 8
                                        Repeater {
                                            model: index === 0 ? [] : ["📄 /Downloads/tax_return_2023.pdf", "📄 /Documents/Invoice_AWS.pdf"]
                                            Rectangle { width: citL.implicitWidth + 20; height: 26; radius: 13; color: "#252525"; border.color: "#444"
                                                Label { id: citL; anchors.centerIn: parent; text: modelData; color: "#888"; font.pixelSize: 11; font.family: "Consolas" }
                                            }
                                        }
                                    }

                                    // ── Suggested Actions ──
                                    RowLayout {
                                        visible: index === 1; spacing: 10
                                        Button { text: "🔒 Lock Invoices in Secure Vault"
                                            background: Rectangle { color: "#1a4a1a"; radius: 6; border.color: "#4caf50" }
                                            contentItem: Label { text: parent.text; color: "#4caf50"; font.bold: true } }
                                        Button { text: "✂️ Move to /Financials"; background: Rectangle { color: "#252525"; radius: 6; border.color: "#444" }; contentItem: Label { text: parent.text; color: "white" } }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Input Footer ──────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true; height: 50; radius: 25; color: "#161616"; border.color: "#333"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 10; spacing: 10
                
                Button { text: "📎"; flat: true; contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 16 } }
                
                TextInput {
                    Layout.fillWidth: true; color: "white"; font.pixelSize: 14
                    placeholderText: "Ask about your files or request an automation..."
                }

                Button { text: "🎙️"; flat: true; contentItem: Label { text: parent.text; color: "#888"; font.pixelSize: 16 } }
                
                Button {
                    text: "↑"
                    background: Rectangle { width: 34; height: 34; color: "#0078d4"; radius: 17 }
                    contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 18; font.bold: true; horizontalAlignment: Text.AlignHCenter }
                }
            }
        }
    }
}
