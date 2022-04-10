import QtQuick 2.15
import QtQuick.Controls 2.15

AutoSizingMenu {
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
