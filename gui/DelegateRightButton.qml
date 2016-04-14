import QtQuick 2.5
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

MMImage {
    property var modeldata

    source: guiblocked == 2 ? "/icons/trash_press.png" : "/icons/trash.png"
    mmwidth: 7
    mmheight: 7
    visible: (modeldata.modstate === ESModElement.InstalledHasUpdate)

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (modeldata.guiblocked === ESModElement.NoBlock)
            {
                var operationModel = mainDelegateContainer.ListView.view.model
                operationModel.Delete(index)
            }
        }
    }
}
