import QtQuick 2.15
import org.salieff.esmodinstaller 1.0


Item {
    property var delegateRectangle: parent
    anchors.fill: parent

    Rectangle {
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: delegateRectangle.border.width
        }
        width: (delegateRectangle.width - delegateRectangle.border.width * 2) * model.progress / 100
        height: delegateRectangle.height - delegateRectangle.border.width * 2
        radius: delegateRectangle.radius - delegateRectangle.border.width
        color: {
            switch (model.modstate) {
            case ESModElement.Unknown :
                "darkslategrey"
                break;

            case ESModElement.Available :
            case ESModElement.Downloading :
                "#0000e0"
                break;

            case ESModElement.Unpacking :
            case ESModElement.InstalledAvailable :
            case ESModElement.InstalledHasUpdate :
                "#008000"
                break;

            case ESModElement.Failed :
                "red"
                break;

            case ESModElement.Installed :
                "#608060"
                break;
            }
        }

        visible: (model.modstate !== ESModElement.Unknown)
    }

    Rectangle {
        anchors.centerIn: parent
        width: delegateRectangle.width - delegateRectangle.border.width
        height: delegateRectangle.height - delegateRectangle.border.width
        radius: delegateRectangle.radius - delegateRectangle.border.width/2

        border.width: delegateRectangle.border.width/2
        border.color: "#22FFFFFF"

        gradient: Gradient {
            GradientStop { position: 0;    color: "#88FFFFFF" }
            GradientStop { position: .1;   color: "#55FFFFFF" }
            GradientStop { position: .5;   color: "#33FFFFFF" }
            GradientStop { position: .501; color: "#11000000" }
            GradientStop { position: .8;   color: "#11FFFFFF" }
            GradientStop { position: 1;    color: "#55FFFFFF" }
        }

        visible: (model.modstate !== ESModElement.Unknown)
    }
}
