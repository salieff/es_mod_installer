import QtQuick 2.3
import QtQuick.Layouts 1.2

Rectangle {
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width - margin * 2;
    height: Math.max(delegateText.implicitHeight, delegateImage.implicitHeight) + 2 * margin
    radius: 15
    border.width: 2
    border.color: "#22000000"
    color: "darkslategrey"

    property int margin: 10

    Rectangle {
        property int koeff: (modstate == "Downloading" || modstate == "Unpacking") ? progress : 100
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: parent.border.width
        }
        width: (parent.width - parent.border.width * 2) * koeff / 100
        height: parent.height - parent.border.width * 2
        radius: parent.radius - parent.border.width
        color: {
            if (modstate == "Unknown")
                "darkslategrey"
            else if (modstate == "Available")
                "#0000e0"
            else if (modstate == "Downloading")
                "#0000e0"
            else if (modstate == "Unpacking")
                "#008000"
            else if (modstate == "Failed")
                "red"
            else if (modstate == "InstalledAvailable")
                "#008000"
            else if (modstate == "InstalledHasUpdate")
                "#008000"
            else if (modstate == "Installed")
                "#608060"
        }

        visible: (modstate != "Unknown") ? true : false
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width - parent.border.width
        height: parent.height - parent.border.width
        radius: parent.radius - parent.border.width/2

        border.width: parent.border.width/2
        border.color: "#22FFFFFF"

        gradient: Gradient {
            GradientStop { position: 0;    color: "#88FFFFFF" }
            GradientStop { position: .1;   color: "#55FFFFFF" }
            GradientStop { position: .5;   color: "#33FFFFFF" }
            GradientStop { position: .501; color: "#11000000" }
            GradientStop { position: .8;   color: "#11FFFFFF" }
            GradientStop { position: 1;    color: "#55FFFFFF" }
        }

        visible: (modstate != "Unknown") ? true : false
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: parent.margin
        spacing: 10

        Image {
            id: delegateImage
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
            source: {
                if (modstate == "Unknown")
                    "icons/download.png"
                else if (modstate == "Available")
                    "icons/download.png"
                else if (modstate == "Downloading")
                    "icons/abort.png"
                else if (modstate == "Unpacking")
                    "icons/abort.png"
                else if (modstate == "Failed")
                    "icons/reload.png"
                else if (modstate == "InstalledAvailable")
                    "icons/trash.png"
                else if (modstate == "InstalledHasUpdate")
                    "icons/update.png"
                else if (modstate == "Installed")
                    "icons/trash.png"
            }

            visible: (modstate != "Unknown") ? true : false

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (modstate == "Available")
                        esModel.Download(index)
                    else if (modstate == "Downloading" || modstate == "Unpacking")
                        esModel.Abort(index)
                    else if (modstate == "Failed")
                        esModel.Retry(index)
                    else if (modstate == "InstalledAvailable")
                        esModel.Delete(index)
                    else if (modstate == "InstalledHasUpdate")
                        esModel.Update(index)
                    else if (modstate == "Installed")
                        esModel.Delete(index)
                }
            }
        }

        Text {
            id: delegateText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 35
            wrapMode: Text.Wrap
            style: Text.Sunken
            color: "white"
            styleColor: "black"
            text: title
        }

        Image {
            id: delegateImage2
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
            }
            source: "icons/trash.png"
            visible: (modstate == "InstalledHasUpdate") ? true : false
            MouseArea {
                anchors.fill: parent
                onClicked: esModel.Delete(index)
            }
        }
    }
}
