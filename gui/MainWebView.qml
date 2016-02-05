import QtQuick 2.3
import QtWebView 1.0

WebView {
    property ListView view

    visible: false

    anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
        margins: 5
    }

    function hide() {
        if (visible) {
            visible = false
            view.enabled = true
            return true
        }
        return false
    }

    function show() {
        if (visible)
            return false

        view.enabled = false
        visible = true
        return true
    }

    Connections {
        target: view
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
