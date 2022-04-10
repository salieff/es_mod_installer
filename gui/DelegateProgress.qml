import QtQuick 2.15
import org.salieff.esmodinstaller 1.0

Item {
    anchors.fill: parent

    property var modeldata
    property Rectangle outrect: parent

    Rectangle {
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: outrect.border.width
        }
        width: (parent.width - outrect.border.width * 2) * modeldata.progress / 100
        height: parent.height - outrect.border.width * 2
        radius: outrect.radius - outrect.border.width
        color: {
            switch (modeldata.modstate) {
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

        visible: (modeldata.modstate !== ESModElement.Unknown)
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width - outrect.border.width
        height: parent.height - outrect.border.width
        radius: parent.radius - outrect.border.width/2

        border.width: outrect.border.width/2
        border.color: "#22FFFFFF"

        gradient: Gradient {
            GradientStop { position: 0;    color: "#88FFFFFF" }
            GradientStop { position: .1;   color: "#55FFFFFF" }
            GradientStop { position: .5;   color: "#33FFFFFF" }
            GradientStop { position: .501; color: "#11000000" }
            GradientStop { position: .8;   color: "#11FFFFFF" }
            GradientStop { position: 1;    color: "#55FFFFFF" }
        }

        visible: (modeldata.modstate !== ESModElement.Unknown)
    }
}
