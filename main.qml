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

            Rectangle {
                id: infoText
                objectName: "infoText"
                anchors.fill: parent;
                anchors {
                    leftMargin: 5
                    rightMargin: 5
                }
                radius: 10
                opacity: 0
                visible: false
                onOpacityChanged: if (opacity == 0) { visible = false }

                Text {
                    anchors.fill: parent;
                    anchors.margins: 10
                    font.pointSize: 18
                    wrapMode: Text.Wrap
                    text: "This property holds whether the item is visible. By default this is true.
Setting this property directly affects the visible value of child items. When set to false, the visible values of all child items also become false. When set to true, the visible values of child items are returned to true, unless they have explicitly been set to false.
(Because of this flow-on behavior, using the visible property may not have the intended effect if a property binding should only respond to explicit property changes. In such cases it may be better to use the opacity property instead.)
If this property is set to false, the item will no longer receive mouse events, but will continue to receive key events and will retain the keyboard focus if it has been set. (In contrast, setting the enabled property to false stops both mouse and keyboard events, and also removes focus from the item.)"
                }
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
                    text: "ES mod manager"
                }

                Image {
                    id: helpImage
                    source: "icons/info.png"
                    sourceSize.width: Screen.pixelDensity * 6
                    sourceSize.height: Screen.pixelDensity * 6
                    MouseArea {
                        anchors.fill: parent
                        PropertyAnimation {id:fadein; target: infoText; property: "opacity"; from: 0; to: 0.95; duration: 300}
                        PropertyAnimation {id:fadeout; target: infoText; property: "opacity"; from: 0.95; to: 0; duration: 300}
                        onClicked: {
                            if (infoText.visible) {
                                fadeout.start()
                            }
                            else {
                                infoText.visible = true
                                fadein.start()
                            }
                        }
                    }
                }
            }
        }
    }
}
