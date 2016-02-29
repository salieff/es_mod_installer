import QtQuick 2.3
import QtWebView 1.0

WebView {
    visible: false

    anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
        margins: 5
    }

    height: parent.height - mainAppTitle.height

    function hide() {
        if (visible) {
            visible = false
            // stop()
            mainListView.enabled = true
            return true
        }
        return false
    }

    function show() {
        // if (visible)
        //    return false

        mainWindow.hideAllPanels()

        mainListView.enabled = false
        visible = true
        mainAppTitle.closeButton.state = "CLOSE"
        return true
    }

    Connections {
        target: mainListView
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
}
