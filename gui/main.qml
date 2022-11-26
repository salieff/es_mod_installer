import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15


ApplicationWindow {
    id: mainWindow
    width: 720
    height: 900
    title: qsTr("ES Manager")
    visible: true

    function mm(i) {
        return Screen.pixelDensity * i
    }

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
        anchors.bottom: parent.bottom

        function isVisible() {
            if (item)
                return item.visible
            else
                return false
        }

        function getUrl() {
            if (item)
                return item.url
            else
                return ""
        }

        function hide() {
            if (!item)
                source = "MainWebView.qml"

            return item.hide()
        }

        function show(url_for_show) {
            if (!item)
                source = "MainWebView.qml"

            return item.show(url_for_show)
        }
    }

    MainLikePanel {
        id: mainLikePanel
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
        if (mainInfoPanel.hide() || mainWebView.hide() || mainLikePanel.hide() || mainTracebackPanel.hide() || mainButtonSelector.activeFocus == false)
        {
            mainAppTitle.closeButton.state = "NORMAL"
            mainButtonSelector.forceActiveFocus()
            return true
        }

        return false
    }
}
