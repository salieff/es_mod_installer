import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0


MMImage {
    source: {
        switch (model.modstate) {
        case ESModElement.Unknown :
            "/icons/info.png"
            break;

        case ESModElement.Available :
            model.guiblocked === ESModElement.ByDownload ? "/icons/download_press.png" : "/icons/download.png"
            break;

        case ESModElement.Downloading :
        case ESModElement.Unpacking :
            model.guiblocked === ESModElement.ByAbort ? "/icons/abort_press.png" : "/icons/abort.png"
            break;

        case ESModElement.Failed :
            (model.guiblocked === ESModElement.ByRetry || model.guiblocked === ESModElement.ByDownload) ? "/icons/reload_press.png" : "/icons/reload.png"
            break;

        case ESModElement.InstalledAvailable :
        case ESModElement.Installed :
            model.guiblocked === ESModElement.ByDelete ? "/icons/trash_press.png" : "/icons/trash.png"
            break;

        case ESModElement.InstalledHasUpdate :
            model.guiblocked === ESModElement.ByUpdate ? "/icons/update_press.png" : "/icons/update.png"
        }
    }

    mmwidth: 7
    mmheight: 7

    visible: (model.modstate !== ESModElement.Unknown)

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (model.guiblocked === ESModElement.NoBlock) {
                let operationModel = mainDelegateContainer.ListView.view.model
                switch (model.modstate) {
                case ESModElement.Available :
                    operationModel.Download(index)
                    break;

                case ESModElement.Downloading :
                case ESModElement.Unpacking :
                    operationModel.Abort(index)
                    break;

                case ESModElement.Failed :
                    operationModel.Retry(index)
                    break;

                case ESModElement.InstalledAvailable :
                case ESModElement.Installed :
                    operationModel.Delete(index)
                    break;

                case ESModElement.InstalledHasUpdate :
                    operationModel.Update(index)
                    break;
                }
            }
        }
    }
}
