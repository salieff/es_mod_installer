import QtQuick 2.5
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

MMImage {
    property var modeldata

    source: {
        switch (modeldata.modstate) {
        case ESModElement.Unknown :
            "/icons/info.png"
            break;

        case ESModElement.Available :
            modeldata.guiblocked === ESModElement.ByDownload ? "/icons/download_press.png" : "/icons/download.png"
            break;

        case ESModElement.Downloading :
        case ESModElement.Unpacking :
            modeldata.guiblocked === ESModElement.ByAbort ? "/icons/abort_press.png" : "/icons/abort.png"
            break;

        case ESModElement.Failed :
            (modeldata.guiblocked === ESModElement.ByRetry || modeldata.guiblocked === ESModElement.ByDownload) ? "/icons/reload_press.png" : "/icons/reload.png"
            break;

        case ESModElement.InstalledAvailable :
        case ESModElement.Installed :
            modeldata.guiblocked === ESModElement.ByDelete ? "/icons/trash_press.png" : "/icons/trash.png"
            break;

        case ESModElement.InstalledHasUpdate :
            modeldata.guiblocked === ESModElement.ByUpdate ? "/icons/update_press.png" : "/icons/update.png"
        }
    }

    mmwidth: 7
    mmheight: 7

    visible: (modeldata.modstate !== ESModElement.Unknown)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (modeldata.guiblocked === ESModElement.NoBlock) {
                switch (modeldata.modstate) {
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
