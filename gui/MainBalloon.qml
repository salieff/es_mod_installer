import QtQuick 2.15

Rectangle {
    z: 1
    width: balloonText.contentWidth + 20
    height: balloonText.contentHeight + 20
    anchors.centerIn: parent
    radius: 10
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
        width: mainWindow.width - 30
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
        onBalloonText: {
            balloonText.text = text
            visible = true
            opacityAnim.restart()
        }
    }

}
