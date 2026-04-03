import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// Taskbar.qml — High-fidelity Windows 11 style taskbar for bobfilez shell.
/// Features centered app icons, glassmorphism (Acrylic/Mica), and a system tray.

Rectangle {
    id: taskbar
    width: parent.width; height: 48
    anchors.bottom: parent.bottom
    color: "#cc1a1a1a" // Translucent dark
    
    // Acrylic effect (requires bobui / QtGraphicalEffects)
    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        color: "#80000000"
        radius: 8
        samples: 17
        verticalOffset: -2
    }

    // Border for top edge
    Rectangle {
        width: parent.width; height: 1
        anchors.top: parent.top
        color: "#33ffffff"
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12; anchors.rightMargin: 12
        spacing: 0

        // ── Left: Widgets / Desktops (Optional) ───────────────────────────
        RowLayout {
            spacing: 8
            Item { width: 40; height: 40
                Label { anchors.centerIn: parent; text: "🌩️"; font.pixelSize: 20 }
                MouseArea { anchors.fill: parent; hoverEnabled: true; onEntered: parent.opacity = 0.7; onExited: parent.opacity = 1.0 }
            }
            NexusPulse { id: sysPulse }
        }

        Item { Layout.fillWidth: true }

        // ── Center: Pinned & Active Apps (Windows 11 Style) ──────────────
        RowLayout {
            spacing: 4
            
            // Start Button
            Rectangle {
                width: 40; height: 40; radius: 4
                color: startHover.hovered ? "#22ffffff" : "transparent"
                Label { anchors.centerIn: parent; text: "📁"; font.pixelSize: 24 } // bobfilez logo here
                HoverHandler { id: startHover }
                MouseArea { anchors.fill: parent; onClicked: shell.toggleStartMenu() }
            }

            // Search Button
            Rectangle {
                width: 40; height: 40; radius: 4
                color: searchHover.hovered ? "#22ffffff" : "transparent"
                Label { anchors.centerIn: parent; text: "🔍"; font.pixelSize: 20; color: "#0078d4" }
                HoverHandler { id: searchHover }
                MouseArea { anchors.fill: parent; onClicked: shell.openPanel("search") }
            }

            // OmniOracle Button (AI Copilot)
            Rectangle {
                width: 40; height: 40; radius: 4
                color: oracleHover.hovered ? "#22ffffff" : "transparent"
                Label { anchors.centerIn: parent; text: "🤖"; font.pixelSize: 20; color: "#0078d4" }
                HoverHandler { id: oracleHover }
                MouseArea { anchors.fill: parent; onClicked: shell.openPanel("oracle") }
            }

            // Task View
            Rectangle {
                width: 40; height: 40; radius: 4
                color: taskHover.hovered ? "#22ffffff" : "transparent"
                Label { anchors.centerIn: parent; text: "📑"; font.pixelSize: 18 }
                HoverHandler { id: taskHover }
            }

            Rectangle { width: 1; height: 24; color: "#33ffffff"; Layout.leftMargin: 4; Layout.rightMargin: 4 }

            // Pinned Apps Repeater
            Repeater {
                model: [
                    {icon: "🌐", color: "#4285f4", active: true},
                    {icon: "✉️", color: "#ea4335", active: false},
                    {icon: "📝", color: "#34a853", active: false},
                    {icon: "📷", color: "#fbbc05", active: false},
                    {icon: "🎬", color: "#ff0000", active: true}
                ]
                Rectangle {
                    width: 40; height: 40; radius: 4
                    color: appHover.hovered ? "#22ffffff" : "transparent"
                    Label { anchors.centerIn: parent; text: modelData.icon; font.pixelSize: 22 }
                    
                    // Active indicator line
                    Rectangle {
                        anchors.bottom: parent.bottom; anchors.bottomMargin: 2
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: modelData.active ? 12 : 0; height: 3; radius: 1.5
                        color: "#0078d4"
                        Behavior on width { NumberAnimation { duration: 200 } }
                    }

                    HoverHandler { id: appHover }
                }
            }
        }

        Item { Layout.fillWidth: true }

        // ── Right: System Tray ───────────────────────────────────────────
        RowLayout {
            spacing: 2

            // Hidden icons
            Label { text: "︿"; color: "#888"; font.pixelSize: 12; Layout.rightMargin: 8 }

            // Status icons (clickable group)
            Rectangle {
                width: trayRow.implicitWidth + 12; height: 34; radius: 17
                color: trayHover.hovered ? "#22ffffff" : "transparent"
                RowLayout {
                    id: trayRow; anchors.centerIn: parent; spacing: 8
                    Label { text: "📶"; font.pixelSize: 14; color: "white" }
                    Label { text: "🔊"; font.pixelSize: 14; color: "white" }
                    Label { text: "🔋"; font.pixelSize: 14; color: "white" }
                }
                HoverHandler { id: trayHover }
            }

            // Date & Time
            Rectangle {
                width: timeCol.implicitWidth + 16; height: 34; radius: 4
                color: timeHover.hovered ? "#22ffffff" : "transparent"
                Column {
                    id: timeCol; anchors.centerIn: parent
                    Label { text: Qt.formatTime(new Date(), "h:mm AP"); color: "white"; font.pixelSize: 11; horizontalAlignment: Text.AlignRight }
                    Label { text: Qt.formatDate(new Date(), "M/d/yyyy"); color: "white"; font.pixelSize: 10; horizontalAlignment: Text.AlignRight }
                }
                HoverHandler { id: timeHover }
            }

            // Notifications / Desktop button
            Rectangle {
                width: 5; height: 40; anchors.right: parent.right
                color: "transparent"
                Rectangle { anchors.right: parent.right; width: 1; height: 40; color: "#33ffffff" }
            }
        }
    }
}
