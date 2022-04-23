import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0


MMImage {
    source: model.guiblocked === ESModElement.ByDelete ? "/icons/trash_press.png" : "/icons/trash.png"
    mmwidth: 7
    mmheight: 7
    visible: ((model.modstate === ESModElement.InstalledHasUpdate) ||
              ((model.modstate === ESModElement.Available || model.modstate === ESModElement.Failed) && model.progress !== 100))

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (model.guiblocked === ESModElement.NoBlock)
            {
                let operationModel = mainDelegateContainer.ListView.view.model
                operationModel.Delete(index)
            }
        }
    }
}
