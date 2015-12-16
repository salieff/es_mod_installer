import QtQuick 2.3
import QtQuick.Controls 1.2

ApplicationWindow {
    visible: true
    width: 720
    height: 900
    title: qsTr("ES Manager")

    /*
    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("&Open")
                onTriggered: console.log("Open action triggered");
            }
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
    }
    */

    ListView {
        model: esModel
        delegate: Delegate {}
        anchors.fill: parent
        anchors.margins: 10
        // clip: true
        spacing: 5

        remove: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
            NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
        }

        populate: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
        }

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
        }

        BusyIndicator {
            objectName: "viewBusyIndicator"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.margins: 40
            width: 200
            height: 200
            running: false;
        }
    }
}

