import QtQuick 2.3
import QtWebView 1.0

WebView {
    property ListView view
    property MMImage closeButton

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
        closeButton.state = "CLOSE"
        return true
    }

    Connections {
        target: view
        onInfoUriSignal: {
            if (uriStr.indexOf("qrc:/") == 0)
            {
                show()
                var xhr = new XMLHttpRequest;
                xhr.open("GET", uriStr);
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === XMLHttpRequest.DONE) {
                        loadHtml(xhr.responseText, "qrc:/help")
                    }
                }
                xhr.send();
            }
            else if (uriStr)
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
