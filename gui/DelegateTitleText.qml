import QtQuick 2.15
import QtQuick.Layouts 1.15


Item {
    id: outRect
    Layout.fillWidth: true
    Layout.preferredHeight: rowLayout.implicitHeight

    property alias fontSize: textLine.font.pointSize
    property alias text: textLine.text

    signal clicked(var mouse)
    signal pressAndHold(var mouse)

    RowLayout {
        id: rowLayout
        anchors.fill: parent

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
        }

        Text {
            id: textLine

            Layout.maximumWidth: outRect.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            style: Text.Sunken
            color: "white"
            styleColor: "black"

            MouseArea {
                id: mouseArea
                anchors.fill: parent

                onContainsPressChanged: {
                    if (containsPress)
                        textLine.color = "red"
                    else
                        textLine.color = "white"
                }

                onClicked: outRect.clicked(mouse)
                onPressAndHold: outRect.pressAndHold(mouse)
            }
        }

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
        }
    }
}
