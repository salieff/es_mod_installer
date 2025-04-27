import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


ListView {
    property string headerText
    delegate: Delegate {}

    spacing: mm(0.3)
    maximumFlickVelocity: 7000

    ScrollBar.vertical: EsScrollBar {}

    header: Item {
        // Empty placeholder
        width: parent.width
        height: hdrRect.height + mm(1)
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
        height: hdrRect.height + mm(1)
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.75

        Rectangle {
            id: hdrRect
            width: hdrText.contentWidth + mm(1)
            height: hdrText.contentHeight + mm(1)
            anchors.horizontalCenter: parent.horizontalCenter
            radius: mm(1)
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
