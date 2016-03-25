import QtQuick 2.5
import QtWebView 1.0

WebView {
    visible: false

    /*
    anchors {
        bottom: mainWindow.bottom
        left: mainWindow.left
        right: mainWindow.right
        margins: 5
    }
    */

    width: mainWindow.width
    height: mainWindow.height - mainAppTitle.height

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

    /*
    Connections {
        target: mainWindow
        onInfoUriSignal: {
            if (uriStr)
            {
                show()
                if (url !== uriStr)
                {
                    url = "about:blank"
                    url = uriStr
                }
            }
        }
    }
    */
}
