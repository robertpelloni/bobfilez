import QtQuick 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1100
    height: 800
    title: "Omni Dashboard"

    background: Rectangle {
        color: "#000"
        LinearGradient {
            anchors.fill: parent
            start: Qt.point(0, 0)
            end: Qt.point(width, height)
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#0f172a" }
                GradientStop { position: 1.0; color: "#020617" }
            }
        }
    }

    Item {
        anchors.fill: parent

        Dashboard {
            anchors.centerIn: parent
            width: Math.min(parent.width - 80, 520)
            height: Math.min(parent.height - 80, 760)
        }
    }
}
