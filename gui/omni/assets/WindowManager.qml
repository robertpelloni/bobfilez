import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/// WindowManager.qml — Windows 11 style window orchestration for OmniShell.
/// Handles tiling, snapping (Layout Assist), and multi-window focus.
/// "Ultimate OS Replacement" from the v4.0 project vision.

Item {
    id: windowManager
    anchors.fill: parent

    property var windows: [] // List of active tool windows
    property var activeWindow: null

    /// Snap a window to a specific region (Left half, Right half, etc.)
    function snapWindow(win, region) {
        let margin = 40
        let areaW = parent.width - (margin * 2)
        let areaH = parent.height - 48 - (margin * 2) // Taskbar offset

        switch(region) {
            case "left":
                win.x = margin; win.y = margin; win.width = areaW/2; win.height = areaH; break;
            case "right":
                win.x = margin + areaW/2; win.y = margin; win.width = areaW/2; win.height = areaH; break;
            case "top-left":
                win.x = margin; win.y = margin; win.width = areaW/2; win.height = areaH/2; break;
            case "top-right":
                win.x = margin + areaW/2; win.y = margin; win.width = areaW/2; win.height = areaH/2; break;
            case "full":
                win.x = margin; win.y = margin; win.width = areaW; win.height = areaH; break;
        }
    }

    /// Bring a window to front
    function raiseWindow(win) {
        // Simple z-stack management
        for (let i=0; i < windowArea.children.length; i++) {
            let child = windowArea.children[i];
            if (child === win) child.z = 50;
            else child.z = 10;
        }
        activeWindow = win
    }

    // ── Snap Layout Overlay ──
    Rectangle {
        id: snapPreview
        visible: false
        color: "#330078d4"
        border.color: "#0078d4"; border.width: 2
        radius: 8
        z: 9999
    }
}
