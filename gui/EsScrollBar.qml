import QtQuick
import QtQuick.Controls
import QtQuick.Window


ScrollBar {
    id: modListScrollBar
    minimumSize: 0.07

    contentItem: Rectangle {
        implicitWidth: mm(2)
        implicitHeight: mm(10)
        radius: width / 2
        color: modListScrollBar.pressed ? "black" : "darkslategrey"
        opacity: modListScrollBar.active ? 0.75 : 0

        Behavior on opacity { NumberAnimation { duration: 300 } }
    }
}
