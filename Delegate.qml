import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import org.salieff.esmodinstaller 1.0

Rectangle {
    id: mainRectangle
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width - margin * 2
    height: Math.max(delegateTextBox.Layout.preferredHeight, delegateImage.height, flagBox.height) + margin * 2
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
            switch (modstate) {
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

        visible: (modstate !== ESModElement.Unknown)
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

        visible: (modstate !== ESModElement.Unknown)
    }

    RowLayout {
        id: layout1
        anchors.fill: parent
        anchors.margins: parent.margin
        spacing: parent.margin

        Image {
            id: delegateImage
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            source: {
                switch (modstate) {
                case ESModElement.Unknown :
                    "icons/info.png"
                    break;

                case ESModElement.Available :
                    guiblocked == ESModElement.ByDownload ? "icons/download_press.png" : "icons/download.png"
                    break;

                case ESModElement.Downloading :
                case ESModElement.Unpacking :
                    guiblocked == ESModElement.ByAbort ? "icons/abort_press.png" : "icons/abort.png"
                    break;

                case ESModElement.Failed :
                    (guiblocked == ESModElement.ByRetry || guiblocked == ESModElement.ByDownload) ? "icons/reload_press.png" : "icons/reload.png"
                    break;

                case ESModElement.InstalledAvailable :
                case ESModElement.Installed :
                    guiblocked == ESModElement.ByDelete ? "icons/trash_press.png" : "icons/trash.png"
                    break;

                case ESModElement.InstalledHasUpdate :
                    guiblocked == ESModElement.ByUpdate ? "icons/update_press.png" : "icons/update.png"
                }
            }

            sourceSize.width: Screen.pixelDensity * 9
            sourceSize.height: Screen.pixelDensity * 9
            Layout.preferredWidth: sourceSize.width
            Layout.preferredHeight: sourceSize.height

            visible: (modstate != "Unknown")

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (guiblocked == ESModElement.NoBlock) {
                        switch (modstate) {
                        case ESModElement.Available :
                            esModel.Download(index)
                            break;

                        case ESModElement.Downloading :
                        case ESModElement.Unpacking :
                            esModel.Abort(index)
                            break;

                        case ESModElement.Failed :
                            esModel.Retry(index)
                            break;

                        case ESModElement.InstalledAvailable :
                        case ESModElement.Installed :
                            esModel.Delete(index)
                            break;

                        case ESModElement.InstalledHasUpdate :
                            esModel.Update(index)
                            break;
                        }
                    }
                }
            }
        }

        Item {
            id: delegateTextBox
            Layout.fillWidth: true
            Layout.preferredHeight: layout2.implicitHeight

            ColumnLayout {
                id: layout2
                anchors.fill: parent
                spacing: mainRectangle.margin

                Text {
                    id: delegateText1
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 18
                    wrapMode: Text.Wrap
                    style: Text.Sunken
                    color: "white"
                    styleColor: "black"
                    text: title
                }
                Text {
                    id: delegateText2
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 12
                    wrapMode: Text.Wrap
                    style: Text.Sunken
                    color: "white"
                    styleColor: "black"
                    text: "[" + status + "] {" + getReadableFileSizeString(modsize) + ", " + timestamp + "}"

                    function getReadableFileSizeString(fileSizeInBytes) {
                        var i = -1;
                        var byteUnits = [' kb', ' Mb', ' Gb', ' Tb', 'Pb', 'Eb', 'Zb', 'Yb'];
                        do {
                            fileSizeInBytes = fileSizeInBytes / 1024;
                            i++;
                        } while (fileSizeInBytes > 1024 && i < (byteUnits.length - 1));

                        return Math.max(fileSizeInBytes, 0.1).toFixed(1) + byteUnits[i];
                    }

                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: mainRectangle.ListView.view.infoUriSignal(infouri)
            }
        }

        Item {
            id: flagBox
            Layout.preferredHeight: layout3.implicitHeight
            Layout.preferredWidth: layout3.implicitWidth

            ColumnLayout {
                id: layout3
                anchors.fill: parent
                Image {
                    source: "icons/rus_flag.png"
                    sourceSize.width: Screen.pixelDensity * 5
                    sourceSize.height: Screen.pixelDensity * 3
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    visible: (langs.indexOf("Ru") > -1)
                }
                Image {
                    source: "icons/eng_flag.png"
                    sourceSize.width: Screen.pixelDensity * 5
                    sourceSize.height: Screen.pixelDensity * 3
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    visible: (langs.indexOf("En") > -1)
                }
                Image {
                    source: "icons/spa_flag.png"
                    sourceSize.width: Screen.pixelDensity * 5
                    sourceSize.height: Screen.pixelDensity * 3
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    visible: (langs.indexOf("Spa") > -1)
                }
            }
        }

        Image {
            id: delegateImage2
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            source: guiblocked == 2 ? "icons/trash_press.png" : "icons/trash.png"
            sourceSize.width: Screen.pixelDensity * 9
            sourceSize.height: Screen.pixelDensity * 9
            Layout.preferredWidth: sourceSize.width
            Layout.preferredHeight: sourceSize.height
            visible: (modstate === ESModElement.InstalledHasUpdate)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (guiblocked == ESModElement.NoBlock)
                        esModel.Delete(index)
                }
            }
        }
    }
}
