import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// DesktopIcons.qml — Windows 11 style desktop icon grid.
/// Handles desktop shortcuts, selection, and drag-and-drop.

Item {
    id: desktopIcons
    anchors.fill: parent

    property var icons: [
        {name: "This PC", icon: "💻", path: "filez://this-pc"},
        {name: "Recycle Bin", icon: "🗑️", path: "filez://trash"},
        {name: "Network", icon: "🌐", path: "filez://network"},
        {name: "User Files", icon: "👤", path: "filez://user"},
        {name: "Bobfilez", icon: "📁", path: "C:/"}
    ]

    GridView {
        anchors.fill: parent
        anchors.margins: 10
        cellWidth: 80; cellHeight: 100
        flow: GridView.TopToBottom
        model: desktopIcons.icons

        delegate: Item {
            width: 74; height: 94
            
            Rectangle {
                anchors.fill: parent; radius: 4
                color: (iconHover.hovered || iconArea.pressed) ? "#22ffffff" : "transparent"
                border.color: (iconHover.hovered || iconArea.pressed) ? "#33ffffff" : "transparent"
            }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 4; spacing: 4
                
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: modelData.icon
                    font.pixelSize: 40
                }
                
                Label {
                    Layout.fillWidth: true
                    text: modelData.name
                    color: "white"; font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    
                    style: Text.Outline; styleColor: "black" // Desktop readability
                }
            }

            HoverHandler { id: iconHover }
            
            MouseArea {
                id: iconArea
                anchors.fill: parent
                onDoubleClicked: shell.openFolder(modelData.path)
            }
        }
    }
}
