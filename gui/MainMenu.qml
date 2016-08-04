import QtQuick 2.3
import QtQuick.Controls 1.2

Menu {
    title: qsTr("Main Menu")

    MenuItem {
        visible: Qt.platform.os !== "ios" && Qt.platform.os !== "android"
        text: qsTr("Change mods install path")
        onTriggered: {
            mainWindow.hideAllPanels()
            modsPathDialog.open()
        }
    }

    MenuItem {
        text: qsTr("Traceback")
        onTriggered: esModel.copyTraceback()
    }

    MenuItem {
        text: qsTr("Log")
        onTriggered: esModel.copyTraceback(true)
    }

    MenuItem {
        text: qsTr("Exit")
        onTriggered: Qt.quit()
    }
}
