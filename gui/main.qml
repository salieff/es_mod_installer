import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    width: 720
    height: 900
    title: qsTr("ES Manager")
    visible: true

    signal infoUriSignal(string uriStr)
    signal likeBoxSignal(var operationModel, var model)

    MainBalloon {
        id: mainBalloon
    }

    ColumnLayout {
        anchors.fill: parent

        MainAppTitle {
            id: mainAppTitle
            z: 1
        }

        MainLists {
            id: mainLists

            MMImage {
                id: locationGear
                visible: false // Экспериментальная фича

                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 5
                    rightMargin: 20
                }

                source: "/icons/gear.png"
                mmwidth: 5
                mmheight: 5
                opacity: 0.75

                MouseArea {
                    anchors.fill: parent
                    onClicked: mainInstallLocationPanel.show()
                }
            }
        }

        MainSortSearchBox {
            id: mainSortSearchBox
            z: 1
        }
    }

    MainInfoPanel {
        id: mainInfoPanel
    }

    MainTracebackPanel {
        id: mainTracebackPanel
    }

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
            function onInfoUriSignal(uriStr) {
                if (uriStr)
                {
                    if (mainWebView.status == Loader.Null)
                        mainWebView.sourceComponent = Qt.createComponent("MainWebView.qml")

                    mainWebView.show()
                    if (mainWebView.item.url != uriStr)
                        mainWebView.item.url = uriStr
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

    MainInstallLocationPanel {
        id: mainInstallLocationPanel
    }

    MainMenu {
        id: mainMenu
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
            }

            event.accepted = true;
        }
    }

    onClosing: {
        if (hideAllPanels())
            close.accepted = false
    }

    function hideAllPanels() {
        if (mainInfoPanel.hide() || mainWebView.hide() || mainLikePanel.hide() || mainTracebackPanel.hide() || mainInstallLocationPanel.hide() || mainButtonSelector.activeFocus == false)
        {
            mainAppTitle.closeButton.state = "NORMAL"
            mainButtonSelector.forceActiveFocus()
            return true
        }

        return false
    }
}
