import QtQuick 2.15
import QtWebView 1.15
import QtQuick.Controls 2.15

Item {
    ProgressBar {
        id: internalBrowserProgressBar
        visible: internalBrowser.loading

        width: mainWindow.width
        anchors.bottom: internalBrowser.top

        from: 0
        to: 100
        value: internalBrowser.loadProgress
    }

    WebView {
        id: internalBrowser
        width: mainWindow.width
        height: mainWindow.height - mainAppTitle.height - (internalBrowserProgressBar.visible ? internalBrowserProgressBar.height : 0)
        anchors.bottom: parent.bottom
    }

    property alias url: internalBrowser.url

    function hide() {
        if (visible) {
            visible = false
            // stop()
            mainLists.enabled = true
            return true
        }
        return false
    }

    function show() {
        // if (visible)
        //    return false

        mainWindow.hideAllPanels()

        mainLists.enabled = false
        visible = true
        mainAppTitle.closeButton.state = "CLOSE"
        return true
    }
}
