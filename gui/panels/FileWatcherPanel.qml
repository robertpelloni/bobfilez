import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/// FileWatcherPanel.qml — Real-time directory monitoring UI ("Shadow Sorter")
///
/// Displays live filesystem events (created/modified/deleted/renamed) from
/// the NativeFileWatcher backend. Users can add/remove watch paths, configure
/// debounce, and see events stream in real-time with colored badges.

Rectangle {
    id: watcherPanel
    color: "#1a1a1a"

    // ── State ──────────────────────────────────────────────────────────────
    property bool isWatching: false
    property int debounceMs: 500
    property int eventCount: 0
    property var watchPaths: []
    property var events: ListModel {}

    signal startWatching()
    signal stopWatching()
    signal addPath(string path)
    signal removePath(string path)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // ── Header ────────────────────────────────────────────────────────
        RowLayout {
            spacing: 12
            Label { text: "👁️ Shadow Sorter"; color: "white"; font.pixelSize: 18; font.bold: true }
            Label { text: "Real-time File Watcher"; color: "#888"; font.pixelSize: 12 }
            Item { Layout.fillWidth: true }

            // Status badge
            Rectangle {
                width: statusLabel.implicitWidth + 20; height: 24; radius: 12
                color: watcherPanel.isWatching ? "#10b981" : "#555"
                Label { id: statusLabel; anchors.centerIn: parent
                    text: watcherPanel.isWatching ? "● LIVE" : "○ STOPPED"
                    color: "white"; font.pixelSize: 11; font.bold: true }
            }

            Button {
                text: watcherPanel.isWatching ? "⏹ Stop" : "▶ Start"
                contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 12 }
                background: Rectangle { color: watcherPanel.isWatching ? "#dc2626" : "#10b981"; radius: 6 }
                onClicked: watcherPanel.isWatching ? watcherPanel.stopWatching() : watcherPanel.startWatching()
            }
        }

        // ── Watch Paths ───────────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Watched Directories"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 8

            ColumnLayout {
                anchors.fill: parent; spacing: 6

                RowLayout {
                    spacing: 8
                    TextField {
                        id: newPathField
                        Layout.fillWidth: true; placeholderText: "Add directory path..."
                        color: "white"; font.pixelSize: 12
                        background: Rectangle { color: "#2d2d2d"; radius: 4; border.color: "#555" }
                        onAccepted: {
                            if (text.length > 0) {
                                watcherPanel.addPath(text)
                                text = ""
                            }
                        }
                    }
                    Button {
                        text: "+ Add"
                        contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 11 }
                        background: Rectangle { color: "#0078d4"; radius: 4 }
                        onClicked: {
                            if (newPathField.text.length > 0) {
                                watcherPanel.addPath(newPathField.text)
                                newPathField.text = ""
                            }
                        }
                    }
                }

                // Common quick-add buttons
                RowLayout {
                    spacing: 6
                    Repeater {
                        model: ["~/Downloads", "~/Documents", "~/Desktop", "~/Pictures"]
                        Button {
                            text: modelData; flat: true
                            contentItem: Label { text: "📁 " + parent.text; color: "#0078d4"; font.pixelSize: 11 }
                            onClicked: watcherPanel.addPath(modelData)
                        }
                    }
                }

                // Debounce slider
                RowLayout {
                    spacing: 8
                    Label { text: "Debounce:"; color: "#888"; font.pixelSize: 11 }
                    Slider {
                        id: debounceSlider
                        from: 100; to: 5000; value: watcherPanel.debounceMs; stepSize: 100
                        implicitWidth: 200
                        onValueChanged: watcherPanel.debounceMs = value
                    }
                    Label { text: debounceSlider.value + "ms"; color: "#aaa"; font.pixelSize: 11 }
                }
            }
        }

        // ── Live Event Feed ───────────────────────────────────────────────
        GroupBox {
            label: Label { text: "Live Events (" + watcherPanel.eventCount + ")"; color: "#aaa"; font.bold: true }
            Layout.fillWidth: true
            Layout.fillHeight: true
            background: Rectangle { color: "#1e1e1e"; radius: 6; border.color: "#333" }
            padding: 4

            ListView {
                id: eventList
                anchors.fill: parent
                clip: true
                model: watcherPanel.events
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    width: eventList.width; height: 30
                    color: index % 2 === 0 ? "#1a1a1a" : "#1e1e1e"

                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8

                        // Event type badge
                        Rectangle {
                            width: 80; height: 20; radius: 4
                            color: {
                                switch(model.eventType) {
                                    case "NEW": return "#10b98133"
                                    case "MODIFIED": return "#f5930033"
                                    case "DELETED": return "#dc262633"
                                    case "RENAMED": return "#8b5cf633"
                                    default: return "#33333333"
                                }
                            }
                            border.color: {
                                switch(model.eventType) {
                                    case "NEW": return "#10b981"
                                    case "MODIFIED": return "#f59300"
                                    case "DELETED": return "#dc2626"
                                    case "RENAMED": return "#8b5cf6"
                                    default: return "#555"
                                }
                            }
                            Label {
                                anchors.centerIn: parent
                                text: model.eventType || "UNKNOWN"
                                color: "white"; font.pixelSize: 10; font.bold: true
                            }
                        }

                        // File path
                        Label {
                            text: model.filePath || ""
                            color: "#e0e0e0"; font.pixelSize: 12
                            Layout.fillWidth: true; elide: Text.ElideMiddle
                        }

                        // Timestamp
                        Label {
                            text: model.timestamp || ""
                            color: "#666"; font.pixelSize: 10
                        }
                    }
                }
            }
        }
    }
}
