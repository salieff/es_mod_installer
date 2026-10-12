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

    MainWebView {
        id: mainWebView
    }

    MainLikePanel {
        id: mainLikePanel
    }

    Shortcut {
        sequences: ["Back"]
        onActivated: {
            if (!hideAllPanels())
                Qt.quit()
        }
    }

    onClosing: {
        close.accepted = false
    }

    function hideAllPanels() {
        if (mainInfoPanel.hide() || mainWebView.hide() || mainLikePanel.hide() || mainTracebackPanel.hide())
        {
            mainAppTitle.closeButton.state = "NORMAL"
            return true
        }

        return false
    }
}
