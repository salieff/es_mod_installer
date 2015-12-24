import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

ApplicationWindow {
    visible: true
    width: 720
    height: 900
    title: qsTr("ES Manager")

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        ListView {
            id: mainListView
            Layout.fillWidth: true
            anchors.top: appTitle.bottom
            anchors.bottom: parent.bottom
            model: esModel
            delegate: Delegate {}
            anchors.margins: 10
            spacing: 5
            maximumFlickVelocity: 5000

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

        Rectangle {
            id: appTitle
            anchors.top: parent.top
            anchors.topMargin: -radius
            Layout.fillWidth: true
            Layout.minimumHeight: Math.max(appTitleText.implicitHeight, helpImage.height) + radius*4
            radius: 10

            gradient: Gradient {
                GradientStop { position: 0; color: "#FFFFFF" }
                GradientStop { position: 1; color: "#A0A0A0" }
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text {
                    id: appTitleText
                    objectName: "appTitleText"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 18
                    wrapMode: Text.Wrap
                    style: Text.Raised
                    styleColor: "white"
                    text: qsTr("ES mod manager")
                }

                Image {
                    id: helpImage
                    source: "icons/info.png"
                    sourceSize.width: Screen.pixelDensity * 6
                    sourceSize.height: Screen.pixelDensity * 6
                    MouseArea {
                        anchors.fill: parent
                        PropertyAnimation {id:fadein; target: infoRect; property: "opacity"; from: 0; to: 0.95; duration: 300}
                        PropertyAnimation {id:fadeout; target: infoRect; property: "opacity"; from: 0.95; to: 0; duration: 300}
                        onClicked: {
                            if (infoRect.visible) {
                                fadeout.start()
                            }
                            else {
                                infoRect.visible = true
                                mainListView.enabled = false
                                fadein.start()
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: infoRect
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
        onOpacityChanged: if (opacity == 0) {
                              visible = false
                              mainListView.enabled = true
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
            }
        }
    }

}
