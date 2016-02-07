import QtQuick 2.3
import QtQuick.Controls 1.2

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
    }

    MainSortSearchBox {
        id: sortSearchBox
        menu: sortMenu
    }

    MainInfoPanel {
        id: infoPanel
        view: mainListView
    }

    MainWebView {
        id: infoUriView
        height: parent.height - appTitle.height
        view: mainListView
    }

    MainSortMenu {
        id: sortMenu
    }

    MainLikePanel {
        id: likeRect
        self: likeRect
        view: mainListView
    }

    onClosing: {
        if (infoPanel.hide())
            close.accepted = false

        if (infoUriView.hide())
            close.accepted = false

        if (likeRect.hide())
            close.accepted = false
    }
}
