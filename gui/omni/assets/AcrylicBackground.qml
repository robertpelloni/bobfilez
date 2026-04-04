import QtQuick 2.15

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

    // Lightweight acrylic fallback.
    // We intentionally avoid a blur effect item here to keep the dependency
    // surface smaller while preserving the tinted translucent shell feel.
    Rectangle {
        anchors.fill: parent
        visible: backgroundSource !== null
        color: "#10ffffff"
        opacity: 0.25
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
