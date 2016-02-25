import QtQuick 2.3

Rectangle {
    z: 1
    width: balloonText.implicitWidth + 20
    height: balloonText.implicitHeight + 20
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
        color: "white"
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
