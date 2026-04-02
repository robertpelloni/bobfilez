import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    width: 300; height: 400
    color: "#2c2c2c"
    radius: 12
    border.color: "#444"
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        Label { text: "Omni Dashboard"; color: "white"; font.bold: true; font.pixelSize: 16 }
        
        Rectangle { height: 1; Layout.fillWidth: true; color: "#444" }
        
        // CPU Usage
        ColumnLayout {
            Layout.fillWidth: true
            RowLayout {
                Label { text: "CPU Usage"; color: "#888"; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
                Label { text: "12%"; color: "white"; font.pixelSize: 12 }
            }
            ProgressBar { value: 0.12; Layout.fillWidth: true }
        }
        
        // RAM Usage
        ColumnLayout {
            Layout.fillWidth: true
            RowLayout {
                Label { text: "RAM Usage"; color: "#888"; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
                Label { text: "4.2 / 16 GB"; color: "white"; font.pixelSize: 12 }
            }
            ProgressBar { value: 0.26; Layout.fillWidth: true }
        }
        
        // Storage Cleaned (Bobfilez integration)
        ColumnLayout {
            Layout.fillWidth: true
            RowLayout {
                Label { text: "Space Reclaimed"; color: "#888"; font.pixelSize: 12 }
                Item { Layout.fillWidth: true }
                Label { text: "124 GB"; color: "#38bdf8"; font.bold: true; font.pixelSize: 12 }
            }
            ProgressBar { value: 0.85; Layout.fillWidth: true }
        }
        
        Item { Layout.fillHeight: true }
        
        Button {
            text: "Launch Nexus"
            Layout.fillWidth: true
            onClicked: console.log("Launching Nexus...")
        }
    }
}
