import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0

ColumnLayout {
    property var modeldata
    property var maxTextWidth

    RowLayout {
        id: rowLayout
        Layout.fillWidth: true

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
        }

        Text {
            id: delegateText1
            // Layout.fillWidth: true
            Layout.maximumWidth: maxTextWidth - (rowLayout.spacing * 2)
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 13.5
            wrapMode: Text.Wrap
            style: Text.Sunken
            color: "white"
            styleColor: "black"
            text: modeldata.title

            MouseArea {
                anchors.fill: parent

                onContainsPressChanged: {
                    if (containsPress)
                        delegateText1.color = "red"
                    else
                        delegateText1.color = "white"
                }

                onClicked: mainWindow.infoUriSignal(infouri)
                onPressAndHold: esModel.copyToClipboard(infouri, qsTr("URL was copied into clipboard"))
            }
        }

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
        }
    }

    RowLayout {
        id: rowLayout2
        Layout.fillWidth: true

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
        }

        Text {
            id: delegateText2
            // Layout.fillWidth: true
            Layout.maximumWidth: maxTextWidth - (rowLayout2.spacing * 2)
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 9
            wrapMode: Text.Wrap
            style: Text.Sunken
            color: "white"
            styleColor: "black"
            text: "[" + modeldata.status + "] {" + getReadableFileSizeString(modeldata.modsize) + ", " + modeldata.timestamp + "}" + percentIndicator()

            function getReadableFileSizeString(fileSizeInBytes) {
                let i = -1;
                let byteUnits = [' kb', ' Mb', ' Gb', ' Tb', 'Pb', 'Eb', 'Zb', 'Yb'];
                do {
                    fileSizeInBytes = fileSizeInBytes / 1024;
                    i++;
                } while (fileSizeInBytes > 1024 && i < (byteUnits.length - 1));

                return Math.max(fileSizeInBytes, 0.1).toFixed(1) + byteUnits[i];
            }

            function percentIndicator() {
                if (modeldata.modstate === ESModElement.Downloading ||
                        ((modeldata.modstate === ESModElement.Available || modeldata.modstate === ESModElement.Failed) && modeldata.progress !== 100))
                    return " " + modeldata.progress +"%";

                return "";
            }

            MouseArea {
                anchors.fill: parent

                onContainsPressChanged: {
                    if (containsPress)
                        delegateText2.color = "red"
                    else
                        delegateText2.color = "white"
                }

                onClicked: {
                    if (modeldata.modstate === ESModElement.Failed) {
                        // Просто дёрнуть esModel нельзя, рассчёт идёт по индексу, и должен быть адресован соответствующей модели
                        let operationModel = mainDelegateContainer.ListView.view.model
                        operationModel.ShowError(modeldata.index)
                    }
                }
            }
        }

        Item {
            // spacer item
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Rectangle { anchors.fill: parent; color: "#ffaaaa" } // to visualize the spacer
        }
    }
}
