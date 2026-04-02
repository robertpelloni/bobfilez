import QtQuick 2.15
import Qt5Compat.GraphicalEffects 1.15 // For Qt 6 compatibility layer

Item {
    id: root
    property color tintColor: "#202020"
    property real tintOpacity: 0.6
    property real blurRadius: 30

    Rectangle {
        anchors.fill: parent
        color: tintColor
        opacity: tintOpacity
    }

    // Blurred background simulation
    // In a real shell, this would sample the desktop wallpaper
    FastBlur {
        anchors.fill: parent
        source: backgroundSource
        radius: blurRadius
        visible: backgroundSource !== null
    }

    // Noise Texture Layer
    Image {
        anchors.fill: parent
        source: "assets/noise.png"
        fillMode: Image.Tile
        opacity: 0.03
    }
    
    property var backgroundSource: null
}
