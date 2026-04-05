import QtQuick 2.15
import QtQuick.Layouts 1.15
import Omni.File 1.0

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1400
    height: 900
    title: "Omni Explorer"

    background: Rectangle {
        color: "#101010"
    }

    FileModel {
        id: fileModel
    }

    QtObject {
        id: shell

        function openPanel(name) {
            if (name === "none") {
                Qt.quit()
            }
        }

        function togglePeek(filePath) {
            // Explorer-only profile intentionally omits the full shell peek overlay.
        }
    }

    ExplorerWindow {
        anchors.fill: parent
        anchors.margins: 24
    }
}
