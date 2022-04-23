import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0


ColumnLayout {
    id: columnLayout
    property var maxTextWidth

    DelegateTitleText {
        maxTextWidth: columnLayout.maxTextWidth
        fontSize: 13.5
        text: model.title

        onClicked: mainWindow.infoUriSignal(model.infouri)
        onPressAndHold: esModel.copyToClipboard(model.infouri, qsTr("URL was copied into clipboard"))
    }

    DelegateTitleText {
        maxTextWidth: columnLayout.maxTextWidth
        fontSize: 9
        text: "[" + model.status + "] {" + getReadableFileSizeString(model.modsize) + ", " + model.timestamp + "}" + percentIndicator()

        onClicked: {
            if (model.modstate === ESModElement.Failed) {
                // Просто дёрнуть esModel нельзя, индекс должен быть адресован соответствующей модели
                let operationModel = mainDelegateContainer.ListView.view.model
                operationModel.ShowError(model.index)
            }
        }
    }

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
        if (model.modstate === ESModElement.Downloading ||
                ((model.modstate === ESModElement.Available || model.modstate === ESModElement.Failed) && model.progress !== 100))
            return " " + model.progress +"%";

        return "";
    }
}
