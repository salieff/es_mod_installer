import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import org.salieff.esmodinstaller 1.0

Item {
    property int modstate
    property int guiblocked

    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
    Layout.preferredWidth: img.sourceSize.width
    Layout.preferredHeight: img.sourceSize.height

    Image {
        id: img
        source: {
            switch (parent.modstate) {
            case ESModElement.Unknown :
                "/icons/info.png"
                break;

            case ESModElement.Available :
                parent.guiblocked == ESModElement.ByDownload ? "/icons/download_press.png" : "/icons/download.png"
                break;

            case ESModElement.Downloading :
            case ESModElement.Unpacking :
                parent.guiblocked == ESModElement.ByAbort ? "/icons/abort_press.png" : "/icons/abort.png"
                break;

            case ESModElement.Failed :
                (parent.guiblocked == ESModElement.ByRetry || parent.guiblocked == ESModElement.ByDownload) ? "/icons/reload_press.png" : "/icons/reload.png"
                break;

            case ESModElement.InstalledAvailable :
            case ESModElement.Installed :
                parent.guiblocked == ESModElement.ByDelete ? "/icons/trash_press.png" : "/icons/trash.png"
                break;

            case ESModElement.InstalledHasUpdate :
                parent.guiblocked == ESModElement.ByUpdate ? "/icons/update_press.png" : "/icons/update.png"
            }
        }

        sourceSize.width: Screen.pixelDensity * 7
        sourceSize.height: Screen.pixelDensity * 7

        visible: (parent.modstate != ESModElement.Unknown)

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (parent.parent.guiblocked == ESModElement.NoBlock) {
                    switch (parent.parent.modstate) {
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
}
