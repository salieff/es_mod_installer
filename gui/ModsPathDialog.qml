import QtQuick 2.3
import QtQuick.Dialogs 1.2

FileDialog {
    title: qsTr("Change mods install path")
    selectFolder: true
    sidebarVisible: false
    selectMultiple: false

    onAccepted: {
        close()
        mainButtonSelector.forceActiveFocus()
        esModel.changeModsFolder(modsPathDialog.fileUrl)
    }

    onRejected: {
        close()
        mainButtonSelector.forceActiveFocus()
    }
}
