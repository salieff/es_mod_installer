import QtQuick 2.5
import org.salieff.esmodinstaller 1.0

Item {
    property int percent
    property int modstate
    property int borderwidth
    property int radius

    Rectangle {
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: parent.borderwidth
        }
        width: (parent.width - parent.borderwidth * 2) * parent.percent / 100
        height: parent.height - parent.borderwidth * 2
        radius: parent.radius - parent.borderwidth
        color: {
            switch (parent.modstate) {
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

        visible: (parent.modstate !== ESModElement.Unknown)
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width - parent.borderwidth
        height: parent.height - parent.borderwidth
        radius: parent.radius - parent.borderwidth/2

        border.width: parent.borderwidth/2
        border.color: "#22FFFFFF"

        gradient: Gradient {
            GradientStop { position: 0;    color: "#88FFFFFF" }
            GradientStop { position: .1;   color: "#55FFFFFF" }
            GradientStop { position: .5;   color: "#33FFFFFF" }
            GradientStop { position: .501; color: "#11000000" }
            GradientStop { position: .8;   color: "#11FFFFFF" }
            GradientStop { position: 1;    color: "#55FFFFFF" }
        }

        visible: (parent.modstate !== ESModElement.Unknown)
    }
}
