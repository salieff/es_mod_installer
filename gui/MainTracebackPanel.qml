import QtQuick
import QtQuick.Controls


Rectangle {
    height: parent.height - mainAppTitle.height
    anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
        margins: mm(0.5)
    }
    radius: mm(1)
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
        anchors.margins: mm(1)
        contentHeight: tracebackText.implicitHeight

        ScrollBar.vertical: EsScrollBar {}

        Text {
            id: tracebackText
            width: parent.width
            font.pointSize: 18
            wrapMode: Text.Wrap
            text: qsTr("Traceback will be here")

            Connections {
                target: esModel
                function onTracebackText(text) {
                    tracebackText.text = text
                    show()
                }
            }
        }
    }
}
