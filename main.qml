import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.2

ApplicationWindow {
    visible: true
    width: 720
    height: 900
    title: qsTr("ES Manager")
    ColumnLayout {
        anchors.fill: parent

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
        Rectangle {
            id: appTitle
            Layout.fillWidth: true
            anchors.top: parent.top
            height: appTitleText.implicitHeight
            Text {
                id: appTitleText
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 35
                wrapMode: Text.Wrap
                //style: Text.Sunken
                //color: "white"
                //styleColor: "black"
                text: "Установщик дополнений для Everlasting Summer"
            }
        }


        ListView {
            Layout.fillWidth: true
            anchors.top: appTitle.bottom
            anchors.bottom: parent.bottom
            // height: 800
            // orientation: ListView.Vertical
            model: esModel
            delegate: Delegate {}
            //anchors.fill: parent
            //anchors.bottom: parent.bottom
            anchors.margins: 10
            clip: true
            spacing: 5

            remove: Transition {
                NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
                NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
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
}

