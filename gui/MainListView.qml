import QtQuick 2.3
import QtQuick.Controls 1.2

ListView {
    model: esModel
    delegate: Delegate {}
    anchors {
        left: parent.left
        right: parent.right
        margins: 10
    }
    spacing: 5
    // clip: true
    maximumFlickVelocity: 5000

    signal infoUriSignal(string uriStr)
    signal likeBoxSignal(var modeldata)

    remove: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
        NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
    }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
    }

    BusyIndicator {
        id: viewBusyIndicator
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.margins: 40
        width: 200
        height: 200
        running: true;

        Connections {
            target: esModel
            onEsIndexReceived: viewBusyIndicator.running = false;
        }
    }

}
