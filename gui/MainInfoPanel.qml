import QtQuick 2.3

Rectangle {
    property ListView view

    height: parent.height - appTitle.height
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
                          view.enabled = true
                      }

    function hide() {
        if (visible) {
            opacity = 0
            return true
        }
        return false
    }

    function show() {
        if (visible)
            return false

        view.enabled = false
        visible = true
        opacity = 0.95
        return true
    }

    function toggle() {
        if (hide())
            return
        show()
    }

    Flickable {
        anchors.fill: parent;
        anchors.margins: 10
        contentHeight: helpText.implicitHeight

        Text {
            id: helpText
            objectName: "helpText"
            width: parent.width
            font.pointSize: 18
            wrapMode: Text.Wrap
            text: qsTr("<h3>Hello!</h3><br><p>If you see this text instead of help so you don't have internet connection to download help topic!</p>")

            Connections {
                target: esModel
                onAppHelpReceived: helpText.text = text
            }
        }
    }
}
