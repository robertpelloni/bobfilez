import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent
    
    GridView {
        anchors.fill: parent
        anchors.margins: 20
        cellWidth: 100
        cellHeight: 100
        flow: GridView.FlowTopToBottom
        
        model: [
            { name: "This PC", icon: "💻" },
            { name: "Recycle Bin", icon: "🗑️" },
            { name: "Bobfilez", icon: "✨" },
            { name: "Network", icon: "🌐" },
            { name: "Control Panel", icon: "⚙️" }
        ]
        
        delegate: Column {
            spacing: 8
            width: 80
            
            Rectangle {
                width: 64; height: 64
                color: mouseArea.containsMouse ? "#20ffffff" : "transparent"
                radius: 4
                anchors.horizontalCenter: parent.horizontalCenter
                
                Label {
                    anchors.centerIn: parent
                    text: modelData.icon
                    font.pixelSize: 40
                }
                
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onDoubleClicked: {
                        if (modelData.name === "Bobfilez") {
                            // TODO: Open Explorer
                        }
                    }
                }
            }
            
            Label {
                text: modelData.name
                color: "white"
                font.pixelSize: 12
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                style: Text.Outline; styleColor: "black"
            }
        }
    }
}
