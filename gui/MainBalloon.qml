import QtQuick


Rectangle {
    z: 1
    width: balloonText.contentWidth + mm(2)
    height: balloonText.contentHeight + mm(2)
    anchors.centerIn: parent
    radius: mm(1)
    color: "black"
    opacity: 0
    visible: false

    onOpacityChanged: if (opacity == 0) { visible = false }

    Text {
        id: balloonText
        anchors.centerIn: parent
        font.pointSize: 18
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: "white"
        wrapMode: Text.Wrap
        width: mainWindow.width - mm(3)
    }

    SequentialAnimation on opacity {
        id: opacityAnim
        running: false
        loops: 1
        NumberAnimation { from: 0.01; to: 1.0; duration: 1000; easing.type: Easing.InOutQuad }
        PauseAnimation { duration: 1500 }
        NumberAnimation { from: 1.0; to: 0; duration: 1000; easing.type: Easing.InOutQuad }
    }

    Connections {
        target: esModel
        function onBalloonText(text) {
            balloonText.text = text
            visible = true
            opacityAnim.restart()
        }
    }
}
