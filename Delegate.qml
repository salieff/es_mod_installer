import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

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
        property int koeff: progress
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
                    guiblocked == 1 ? "icons/download_press.png" : "icons/download.png"
                else if (modstate == "Available")
                    guiblocked == 1 ? "icons/download_press.png" : "icons/download.png"
                else if (modstate == "Downloading")
                    guiblocked == 1 ? "icons/abort_press.png" : "icons/abort.png"
                else if (modstate == "Unpacking")
                    guiblocked == 1 ? "icons/abort_press.png" : "icons/abort.png"
                else if (modstate == "Failed")
                    guiblocked == 1 ? "icons/reload_press.png" : "icons/reload.png"
                else if (modstate == "InstalledAvailable")
                    guiblocked == 2 ? "icons/trash_press.png" : "icons/trash.png"
                else if (modstate == "InstalledHasUpdate")
                    guiblocked == 1 ? "icons/update_press.png" : "icons/update.png"
                else if (modstate == "Installed")
                    guiblocked == 2 ? "icons/trash_press.png" : "icons/trash.png"
            }

            sourceSize.width: Screen.pixelDensity * 11
            sourceSize.height: Screen.pixelDensity * 11

            visible: (modstate != "Unknown") ? true : false

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (guiblocked == 0) {
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
        }

        Text {
            id: delegateText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 18
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
            source: guiblocked == 2 ? "icons/trash_press.png" : "icons/trash.png"
            sourceSize.width: Screen.pixelDensity * 11
            sourceSize.height: Screen.pixelDensity * 11
            visible: (modstate == "InstalledHasUpdate") ? true : false
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (guiblocked == 0)
                        esModel.Delete(index)
                }
            }
        }
    }
}
