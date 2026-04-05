import QtQuick 2.15
import QtQuick.Window 2.15
import OmniUI 1.0
import OmniLayout 1.0

Window {
    width: 1024
    height: 640
    visible: true
    color: "#10151d"
    title: "bobfilez BobUI Demo"

    Rectangle {
        anchors.fill: parent
        color: "#10151d"

        Column {
            anchors.centerIn: parent
            spacing: 24

            Text {
                text: "bobfilez BobUI Demo"
                color: "#f5f7fa"
                font.pixelSize: 32
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                width: 640
            }

            Text {
                text: "This minimal BobUI frontend uses real OmniUI/omnicore registration on top of the modern Qt QML runtime path."
                color: "#c8d1dc"
                font.pixelSize: 18
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                width: 640
            }

            Text {
                text: "Core version: " + bobfilezVersion
                color: "#7dd3fc"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                width: 640
            }

            Button {
                width: 220
                height: 48
            }
        }
    }
}
