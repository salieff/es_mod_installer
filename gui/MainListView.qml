import QtQuick 2.5
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.2

ListView {
    property string headerText

    delegate: Delegate {}

    Layout.margins: 10
    Layout.fillHeight: true
    Layout.preferredWidth: mainWindow.width - Layout.margins * 2
    spacing: 5
    // clip: true
    maximumFlickVelocity: 5000
    headerPositioning: ListView.OverlayHeader
    header: Item {
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

    remove: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
        NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
    }

    displaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
    }

    Rectangle {
        anchors.left: parent.right
        anchors.leftMargin: 2
        y: parent.visibleArea.yPosition * parent.height
        width: 6
        radius: 2
        height: parent.visibleArea.heightRatio * parent.height
        color: "black"
        opacity: 0.5
    }
}
