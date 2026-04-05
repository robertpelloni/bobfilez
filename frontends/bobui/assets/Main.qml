import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import OmniUI 1.0

Window {
    width: 1120
    height: 780
    visible: true
    color: "#10151d"
    title: "bobfilez BobUI Demo"

    TabBar {
        id: tabBar
        width: parent.width

        TabButton { text: "Dashboard" }
        TabButton { text: "Scanner" }
        TabButton { text: "Duplicates" }
        TabButton { text: "Statistics" }
        TabButton { text: "Hasher" }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex

        Rectangle {
            color: "#10151d"

            Column {
                anchors.centerIn: parent
                spacing: 24
                width: 720

                Text {
                    text: "bobfilez BobUI Demo"
                    color: "#f5f7fa"
                    font.pixelSize: 32
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                Text {
                    text: "This BobUI/QML lane now exposes direct fo_core scan, duplicate, statistics, and hashing workflows through a dedicated QObject bridge."
                    color: "#c8d1dc"
                    font.pixelSize: 18
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                Text {
                    text: "Core version: " + bobfilezVersion
                    color: "#7dd3fc"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }
            }
        }

        Rectangle {
            color: "#10151d"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: scanPathInput
                        Layout.fillWidth: true
                        placeholderText: "Enter absolute directory path to scan..."
                    }

                    Button {
                        text: "Scan Directory"
                        onClicked: {
                            scanStatus.text = "Scanning..."
                            scanModel.clear()
                            foEngine.runScan(scanPathInput.text)
                        }
                    }
                }

                Text {
                    id: scanStatus
                    color: "#c8d1dc"
                    text: "No directory scanned yet."
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: ListModel { id: scanModel }
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 32
                        color: index % 2 === 0 ? "#1e293b" : "#0f172a"

                        Row {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 12

                            Text {
                                text: model.path
                                color: "#e2e8f0"
                                width: parent.width - 180
                                elide: Text.ElideRight
                            }

                            Text {
                                text: model.size + " bytes"
                                color: "#94a3b8"
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            color: "#10151d"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: duplicatesPathInput
                        Layout.fillWidth: true
                        placeholderText: "Enter absolute directory path to find duplicates..."
                    }

                    Button {
                        text: "Find Duplicates"
                        onClicked: {
                            duplicatesStatus.text = "Finding duplicates..."
                            duplicatesModel.clear()
                            foEngine.runDuplicates(duplicatesPathInput.text)
                        }
                    }
                }

                Text {
                    id: duplicatesStatus
                    color: "#c8d1dc"
                    text: "No duplicate search run yet."
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: ListModel { id: duplicatesModel }
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 64
                        color: index % 2 === 0 ? "#1e293b" : "#0f172a"

                        Column {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 4

                            Text {
                                text: "Hash: " + model.hash + " | Size: " + model.size
                                color: "#fbbf24"
                                font.bold: true
                            }

                            Text {
                                text: model.files
                                color: "#e2e8f0"
                                width: parent.width
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            color: "#10151d"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: statsPathInput
                        Layout.fillWidth: true
                        placeholderText: "Enter absolute directory path for statistics..."
                    }

                    Button {
                        text: "Analyze"
                        onClicked: {
                            statsStatus.text = "Analyzing..."
                            statsOutput.text = ""
                            foEngine.runStats(statsPathInput.text)
                        }
                    }
                }

                Text {
                    id: statsStatus
                    color: "#c8d1dc"
                    text: "No statistics gathered yet."
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: statsOutput
                        readOnly: true
                        color: "#e2e8f0"
                        wrapMode: TextArea.Wrap
                        selectionColor: "#1d4ed8"
                        background: Rectangle {
                            color: "#0f172a"
                            radius: 8
                            border.color: "#1e293b"
                        }
                    }
                }
            }
        }

        Rectangle {
            color: "#10151d"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: hashPathInput
                        Layout.fillWidth: true
                        placeholderText: "Enter absolute file path to hash..."
                    }

                    Button {
                        text: "Hash File"
                        onClicked: {
                            hashStatus.text = "Hashing..."
                            hashOutput.text = ""
                            foEngine.runHash(hashPathInput.text)
                        }
                    }
                }

                Text {
                    id: hashStatus
                    color: "#c8d1dc"
                    text: "No file hashed yet."
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: hashOutput
                        readOnly: true
                        color: "#e2e8f0"
                        wrapMode: TextArea.Wrap
                        selectionColor: "#1d4ed8"
                        background: Rectangle {
                            color: "#0f172a"
                            radius: 8
                            border.color: "#1e293b"
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: foEngine

        function onScanFinished(results, stats) {
            scanStatus.text = stats
            scanModel.clear()
            for (var i = 0; i < results.length; ++i) {
                scanModel.append(results[i])
            }
        }

        function onDuplicatesFinished(results, stats) {
            duplicatesStatus.text = stats
            duplicatesModel.clear()
            for (var i = 0; i < results.length; ++i) {
                duplicatesModel.append(results[i])
            }
        }

        function onStatsFinished(report, stats) {
            statsStatus.text = stats
            statsOutput.text = report
        }

        function onHashFinished(report, stats) {
            hashStatus.text = stats
            hashOutput.text = report
        }

        function onErrorOccurred(scope, errorMsg) {
            var message = "Error: " + errorMsg
            if (scope === "scan") {
                scanStatus.text = message
            } else if (scope === "duplicates") {
                duplicatesStatus.text = message
            } else if (scope === "stats") {
                statsStatus.text = message
            } else if (scope === "hash") {
                hashStatus.text = message
            }
        }
    }
}
