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

        if (mainAppTitle.closeButton.pulsed)
        {
            esModel.helpRead(infoText.text)
            mainAppTitle.closeButton.pulsed = false
        }

        mainAppTitle.closeButton.state = "CLOSE"

        return true
    }

    function toggle() {
        if (hide())
            return
        show()
    }

    Flickable {
        id: infoPanelFlickable

        anchors.fill: parent;
        anchors.margins: mm(1)
        contentHeight: infoText.implicitHeight
        maximumFlickVelocity: 7000

        ScrollBar.vertical: EsScrollBar {}

        Text {
            id: infoText
            width: parent.width
            font.pointSize: 18
            wrapMode: Text.Wrap
            text: qsTr("<h3>Hello!</h3><br><p>If you see this text instead of help so you don't have internet connection to download help topic!</p>")

            Connections {
                target: esModel
                function onAppHelpReceived(text, fromServer) {
                    if (infoText.text != text && fromServer)
                        mainAppTitle.closeButton.pulsed = true

                    infoText.text = text
                }
            }
        }
    }
}
