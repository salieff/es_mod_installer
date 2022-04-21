import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    height: parent.height - mainAppTitle.height
    anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
        margins: 5
    }
    radius: 10
    opacity: 0
    visible: false
    clip: true

    Behavior on opacity { NumberAnimation { duration: 300 } }

    onOpacityChanged: if (opacity == 0) {
                          visible = false
                          mainLists.enabled = true
                      }

    function hide() {
        if (visible) {
            opacity = 0
            return true
        }
        return false
    }

    function show() {
        // if (visible)
        //    return false

        mainWindow.hideAllPanels()

        mainLists.enabled = false
        visible = true
        opacity = 0.95
        mainAppTitle.closeButton.state = "CLOSE"

        return true
    }

    function toggle() {
        if (hide())
            return
        show()
    }

    Flickable {
        id: tracebackPanelFlickable

        anchors.fill: parent;
        anchors.margins: 10
        contentHeight: tracebackText.implicitHeight

        ScrollBar.vertical: ScrollBar {
            id: tracebackPanelScrollBar
            minimumSize: 0.07

            parent: tracebackPanelFlickable.parent
            anchors.top: tracebackPanelFlickable.top
            anchors.left: tracebackPanelFlickable.right
            anchors.bottom: tracebackPanelFlickable.bottom

            contentItem: Rectangle {
                implicitWidth: 10
                implicitHeight: 100
                radius: width / 2
                color: tracebackPanelScrollBar.pressed ? "black" : "darkgrey"
            }
        }

        Text {
            id: tracebackText
            width: parent.width
            font.pointSize: 18
            wrapMode: Text.Wrap
            text: qsTr("Traceback will be here")

            Connections {
                target: esModel
                onTracebackText: {
                    tracebackText.text = text
                    show()
                }
            }
        }
    }
}
