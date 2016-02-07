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
        if (infoPanel.hide() || infoUriView.hide() || likeRect.hide())
        {
            close.accepted = false
            appTitle.button.state = "NORMAL"
        }
    }
}
