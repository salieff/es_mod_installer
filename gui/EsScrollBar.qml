import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15


ScrollBar {
    id: modListScrollBar
    minimumSize: 0.07

    contentItem: Rectangle {
        implicitWidth: Screen.pixelDensity * 2
        implicitHeight: 100
        radius: width / 2
        color: modListScrollBar.pressed ? "black" : "darkslategrey"
        opacity: modListScrollBar.active ? 0.75 : 0

        Behavior on opacity { NumberAnimation { duration: 300 } }
    }
}
