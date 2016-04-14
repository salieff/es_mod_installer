import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: mainWindow
    width: 720
    height: 900
    title: qsTr("ES Manager")
    visible: true

    signal infoUriSignal(string uriStr)
    signal likeBoxSignal(var operationModel, var modeldata)

    MainBalloon {
        id: mainBalloon
    }

    MainLists {
        id: mainLists
    }

    MainAppTitle {
        id: mainAppTitle
    }

    MainSortSearchBox {
        id: mainSortSearchBox
    }

    MainInfoPanel {
        id: mainInfoPanel
    }

    MainTracebackPanel {
        id: mainTracebackPanel
    }

    /*
    MainWebView {
        id: mainWebView
    }
    */
    Loader {
        id: mainWebView
        asynchronous: false
        anchors.bottom: parent.bottom

        function hide() {
            if (item)
                return item.hide()
            else
                return false
        }

        function show() {
            if (item)
                return item.show()
            else
                return false
        }

        Connections {
            target: mainWindow
            onInfoUriSignal: {
                if (uriStr)
                {
                    if (mainWebView.status == Loader.Null) {
                        var cmp = Qt.createComponent("MainWebView.qml")
                        mainWebView.sourceComponent = cmp
                    }

                    mainWebView.show()
                    if (mainWebView.item.url !== uriStr)
                    {
                        mainWebView.item.url = "about:blank"
                        mainWebView.item.url = uriStr
                    }
                }
            }
        }
    }

    MainSortMenu {
        id: mainSortMenu
    }

    MainLikePanel {
        id: mainLikePanel
    }

    MainMenu {
        id: mainMenu
    }

    ModsPathDialog {
        id: modsPathDialog
    }

    Connections {
        target: esModel
        onCurrentModsFolder: modsPathDialog.folder = "file://" + newFolder
    }

    Item {
        id: mainButtonSelector
        anchors.fill: parent
        focus: true
        Keys.onPressed: {
            switch(event.key)
            {
            case Qt.Key_Menu :
                mainMenu.popup()
                break;

            case Qt.Key_Back:
                if (!hideAllPanels())
                    Qt.quit()
                break;

            case Qt.Key_1:
                mainLists.state = "SERVER"
                break;

            case Qt.Key_2:
                mainLists.state = "ALL"
                break;

            case Qt.Key_3:
                mainLists.state = "LOCAL"
                break;
            }

            event.accepted = true;
        }
    }

    onClosing: {
        if (hideAllPanels())
            close.accepted = false
    }

    function hideAllPanels() {
        if (mainInfoPanel.hide() || mainWebView.hide() || mainLikePanel.hide() || mainTracebackPanel.hide() || mainButtonSelector.activeFocus == false)
        {
            mainAppTitle.closeButton.state = "NORMAL"
            mainButtonSelector.forceActiveFocus()
            return true
        }

        return false
    }
}
