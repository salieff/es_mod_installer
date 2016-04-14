import QtQuick 2.5
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

ColumnLayout {
    property var modeldata
    property Rectangle outrect: parent.parent

    Layout.fillWidth: true

    Text {
        id: delegateText1
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 13.5
        wrapMode: Text.Wrap
        style: Text.Sunken
        color: "white"
        styleColor: "black"
        text: modeldata.title
    }

    Text {
        id: delegateText2
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 9
        wrapMode: Text.Wrap
        style: Text.Sunken
        color: "white"
        styleColor: "black"
        text: "[" + modeldata.status + "] {" + getReadableFileSizeString(modeldata.modsize) + ", " + modeldata.timestamp + "}"

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

    MouseArea {
        anchors.fill: parent

        onContainsPressChanged: {
            if (containsPress) {
                delegateText1.color = "red"
                delegateText2.color = "red"
            }
            else {
                delegateText1.color = "white"
                delegateText2.color = "white"
            }
        }

        onClicked: {
            if (modeldata.modstate === ESModElement.Failed)
            {
                var operationModel = mainDelegateContainer.ListView.view.model
                operationModel.ShowError(modeldata.index)
            }
            else
            {
                mainWindow.infoUriSignal(infouri)
            }
        }
    }
}
