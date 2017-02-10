import QtQuick 2.5
import QtWebView 1.1

WebView {
    visible: false

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
                if (url != uriStr)
                {
                    url = "about:blank"
                    url = uriStr
                }
            }
        }
    }
    */
}
