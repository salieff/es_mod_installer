import QtQuick 2.15
import QtQuick.Controls 1.4 // An old version is used deliberately, because menus from the Controls 2.15 are ugly

Menu {
    title: qsTr("Main Menu")

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
