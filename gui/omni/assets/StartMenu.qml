import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    width: 600; height: 700
    color: "#2c2c2c"
    radius: 12
    border.color: "#444"
    
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 30; spacing: 20
        
        // Search Box
        Rectangle {
            Layout.fillWidth: true; height: 40; color: "#1c1c1c"; radius: 20; border.color: "#444"
            RowLayout { anchors.fill: parent; anchors.leftMargin: 15; Label { text: "🔍"; color: "#888" }; Label { text: "Type here to search"; color: "#888"; font.pixelSize: 13 } }
        }
        
        Label { text: "Pinned"; color: "white"; font.bold: true; font.pixelSize: 14 }
        
        GridLayout {
            columns: 6; Layout.fillWidth: true; rowSpacing: 20
            Repeater {
                model: ["Edge", "Word", "Excel", "PowerPoint", "Mail", "Calendar", "Photos", "Store", "Settings", "Explorer", "Bobfilez", "Terminal"]
                Column {
                    spacing: 8; Layout.alignment: Qt.AlignHCenter
                    Rectangle { 
                        width: 48; height: 48; color: "#333"; radius: 8
                        Label { anchors.centerIn: parent; text: "📦"; font.pixelSize: 24 }
                        MouseArea { 
                            anchors.fill: parent; hoverEnabled: true
                            onEntered: parent.color = "#444"
                            onExited: parent.color = "#333"
                        }
                    }
                    Label { text: modelData; color: "white"; font.pixelSize: 11; anchors.horizontalCenter: parent.horizontalCenter }
                }
            }
        }
        
        Item { Layout.fillHeight: true }
        
        // User Profile
        Rectangle {
            Layout.fillWidth: true; height: 60; color: "transparent"
            RowLayout {
                anchors.fill: parent
                Rectangle { width: 32; height: 32; radius: 16; color: "#0078d4"; Label { anchors.centerIn: parent; text: "RP"; font.pixelSize: 12; color: "white" } }
                Label { text: "Robert Pelloni"; color: "white"; font.pixelSize: 13 }
                Item { Layout.fillWidth: true }
                Label { text: "⏻"; color: "white"; font.pixelSize: 18 }
            }
        }
    }
}
