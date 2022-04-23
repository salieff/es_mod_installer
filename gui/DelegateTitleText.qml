import QtQuick 2.15
import QtQuick.Layouts 1.15


RowLayout {
    property var maxTextWidth
    property alias fontSize: textLine.font.pointSize
    property alias text: textLine.text

    signal clicked(var mouse)
    signal pressAndHold(var mouse)

    id: rowLayout
    Layout.fillWidth: true

    Item {
        // spacer item
        Layout.fillWidth: true
        Layout.fillHeight: true
        // Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
    }

    Text {
        id: textLine
        Layout.maximumWidth: maxTextWidth - (rowLayout.spacing * 2)
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

            onClicked: rowLayout.clicked(mouse)
            onPressAndHold: rowLayout.pressAndHold(mouse)
        }
    }

    Item {
        // spacer item
        Layout.fillWidth: true
        Layout.fillHeight: true
        // Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
    }
}
