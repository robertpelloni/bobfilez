import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// OmniSharePanel.qml — Zero-Config Secure File Drop System.
/// Share files instantly with auto-expiring encrypted links.

Rectangle {
    id: sharePanel
    color: "#0f0f0f"; radius: 8

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 25; spacing: 20

        RowLayout {
            Label { text: "📤 OmniShare"; font.pixelSize: 22; font.bold: true; color: "white" }
            Label { text: "Secure File Drops (No Cloud Required)"; color: "#888"; font.pixelSize: 14 }
            Item { Layout.fillWidth: true }
            Rectangle { width: 120; height: 24; radius: 12; color: "#1a3a1a"; border.color: "#4caf50"
                Label { anchors.centerIn: parent; text: "HTTPS :8443 LIVE"; color: "#4caf50"; font.pixelSize: 10; font.bold: true } }
        }

        // Drop Zone
        Rectangle {
            Layout.fillWidth: true; height: 120; radius: 10; color: "#161616"; border.color: "#0078d4"; border.width: 2
            Label { anchors.centerIn: parent; text: "📁 Drop Files Here to Create Share Link"; color: "#0078d4"; font.pixelSize: 16; font.bold: true }
        }

        RowLayout {
            spacing: 15
            Label { text: "Max Downloads:"; color: "#888" }
            SpinBox { from: 1; to: 100; value: 5 }
            Label { text: "Expires In:"; color: "#888" }
            ComboBox { model: ["15 min", "1 hour", "24 hours", "7 days", "Never"]; currentIndex: 1 }
            CheckBox { contentItem: Label { text: "Require Password"; color: "#ccc"; leftPadding: 4 } }
        }

        // Active Shares
        GroupBox {
            label: Label { text: "Active Share Links"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true; Layout.fillHeight: true
            background: Rectangle { color: "#111"; radius: 6; border.color: "#333" }
            padding: 10

            ListView {
                anchors.fill: parent; clip: true; spacing: 8
                model: [
                    {file: "presentation_v3.pptx", url: "https://192.168.1.12:8443/share/abc12", dl: "2/5", time: "45 min left", pw: false},
                    {file: "project_source.zip", url: "https://192.168.1.12:8443/share/def34", dl: "0/1", time: "23 hours left", pw: true}
                ]
                delegate: Rectangle {
                    width: parent.width; height: 60; radius: 6; color: "#1e1e1e"; border.color: "#333"
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 12; spacing: 15
                        Label { text: "📤"; font.pixelSize: 18 }
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Label { text: modelData.file; color: "white"; font.bold: true; font.pixelSize: 12 }
                            Label { text: modelData.url; color: "#0078d4"; font.pixelSize: 11; font.family: "Consolas" }
                        }
                        Label { text: modelData.dl; color: "#ccc"; font.pixelSize: 11 }
                        Label { text: modelData.time; color: "#888"; font.pixelSize: 10 }
                        Label { text: modelData.pw ? "🔒" : "🔓"; font.pixelSize: 14 }
                        Button { text: "📋 Copy"; flat: true; contentItem: Label { text: parent.text; color: "#0078d4"; font.pixelSize: 10 } }
                        Button { text: "🗑"; flat: true; contentItem: Label { text: parent.text; color: "#ff4444"; font.pixelSize: 14 } }
                    }
                }
            }
        }
    }
}
