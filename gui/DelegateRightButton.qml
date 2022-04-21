import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0

MMImage {
    property var modeldata

    source: modeldata.guiblocked === ESModElement.ByDelete ? "/icons/trash_press.png" : "/icons/trash.png"
    mmwidth: 7
    mmheight: 7
    visible: ((modeldata.modstate === ESModElement.InstalledHasUpdate) ||
              ((modeldata.modstate === ESModElement.Available || modeldata.modstate === ESModElement.Failed) && modeldata.progress !== 100))

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (modeldata.guiblocked === ESModElement.NoBlock)
            {
                let operationModel = mainDelegateContainer.ListView.view.model
                operationModel.Delete(index)
            }
        }
    }
}
