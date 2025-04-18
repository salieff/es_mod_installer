import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ListView {
    property string headerText
    delegate: Delegate {}

    spacing: 5
    maximumFlickVelocity: 7000

    ScrollBar.vertical: EsScrollBar {}

    header: Item {
        // Empty placeholder
        width: parent.width
        height: hdrRect.height + 10
        anchors.horizontalCenter: parent.horizontalCenter
    }

    remove: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
        NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
    }

    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
    }

    Item {
        z: 2
        width: parent.width
        height: hdrRect.height + 10
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.75

        Rectangle {
            id: hdrRect
            width: hdrText.contentWidth + 20
            height: hdrText.contentHeight + 20
            anchors.horizontalCenter: parent.horizontalCenter
            radius: 10
            gradient: Gradient {
                GradientStop { position: 0; color: "#FFFFFF" }
                GradientStop { position: 1; color: "#A0A0A0" }
            }

            Text {
                id: hdrText
                anchors.centerIn: parent
                font.pointSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                style: Text.Raised
                styleColor: "white"
                text: headerText
            }
        }
    }
}
