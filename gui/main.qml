import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: mainWindow
    width: 720
    height: 900
    title: qsTr("ES Manager")
    visible: true

    MainListView {
        id: mainListView
        anchors {
            top: appTitle.bottom
            bottom: sortSearchBox.top
        }
    }

    MainAppTitle {
        id: appTitle
        infoRect: infoPanel
        webView: infoUriView
        likePanel: likeRect
    }

    MainSortSearchBox {
        id: sortSearchBox
        menu: sortMenu
    }

    MainInfoPanel {
        id: infoPanel
        view: mainListView
        closeButton: appTitle.button
    }

    MainTracebackPanel {
        id: tracebackPanel
        view: mainListView
        closeButton: appTitle.button
    }

    MainWebView {
        id: infoUriView
        height: parent.height - appTitle.height
        closeButton: appTitle.button
        view: mainListView
    }

    MainSortMenu {
        id: sortMenu
    }

    MainLikePanel {
        id: likeRect
        self: likeRect
        closeButton: appTitle.button
        view: mainListView
    }

    onClosing: {
        if (infoPanel.hide() || infoUriView.hide() || likeRect.hide() || tracebackPanel.hide() || buttonSelector.activeFocus == false)
        {
            close.accepted = false
            appTitle.button.state = "NORMAL"
            buttonSelector.forceActiveFocus()
        }
    }

    Menu {
        id: mainMenu
        title: qsTr("Main Menu")

        MenuItem {
            visible: Qt.platform.os !== "ios"
            text: qsTr("Change mods install path")
            onTriggered: {
                if (infoPanel.hide() || infoUriView.hide() || likeRect.hide() || tracebackPanel.hide())
                    appTitle.button.state = "NORMAL"

                fileDialog.open()
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

    FileDialog {
        id: fileDialog
        title: qsTr("Change mods install path")
        selectFolder: true
        sidebarVisible: false
        selectMultiple: false

        onAccepted: {
            console.log("You chose: " + fileDialog.fileUrl)
            fileDialog.close()
            buttonSelector.forceActiveFocus()
            esModel.changeModsFolder(fileDialog.fileUrl)
        }

        onRejected: {
            console.log("Canceled")
            fileDialog.close()
            buttonSelector.forceActiveFocus()
        }
    }

    Connections {
        target: esModel
        onCurrentModsFolder: fileDialog.folder = "file://" + newFolder
    }

    Item {
        id: buttonSelector
        anchors.fill: parent
        focus: true
        Keys.onPressed: {
            switch(event.key)
            {
            case Qt.Key_Menu :
                mainMenu.popup()
                break;

            case Qt.Key_Back:
                if (infoPanel.hide() || infoUriView.hide() || likeRect.hide() || tracebackPanel.hide())
                    appTitle.button.state = "NORMAL"
                else
                    Qt.quit()
                break;
            }

            console.log('ES Key pressed : ' + event.key)
            event.accepted = true;
        }
    }
}
