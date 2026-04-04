import QtQuick 2.15
import QtQuick.Layouts 1.15

/// NexusPulse.qml — Real-time system activity monitor for the Taskbar.
/// Visualizes CPU pressure, Nexus task load, and active I/O.

Rectangle {
    id: pulse
    width: 120; height: 32; radius: 16
    color: "#22ffffff"; border.color: "#33ffffff"

    property real load: 0.15 // 0.0 to 1.0
    property int activeTasks: 2

    RowLayout {
        anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 8
        
        // Animated Pulse Circle
        Rectangle {
            width: 8; height: 8; radius: 4; color: "#0078d4"
            opacity: 1.0
            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { from: 0.3; to: 1.0; duration: 800; easing.type: Easing.InOutSine }
                NumberAnimation { from: 1.0; to: 0.3; duration: 800; easing.type: Easing.InOutSine }
            }
        }

        // Mini Waveform (Simulated CPU/IO)
        Row {
            spacing: 1; Layout.fillWidth: true; Layout.fillHeight: true; anchors.verticalCenter: parent.verticalCenter
            Repeater {
                model: 12
                Rectangle {
                    width: 2; height: 4 + Math.random() * 12; radius: 1; color: "#4caf50"; anchors.verticalCenter: parent.verticalCenter
                    opacity: 0.8
                }
            }
        }

        Text { text: pulse.activeTasks.toString(); color: "white"; font.pixelSize: 10; font.bold: true }
    }
}
