import QtQuick
import QtWebView
import QtQuick.Layouts
import QtQuick.Controls


Rectangle {
    anchors {
        bottom: parent.bottom
        left: parent.left
        right: parent.right
    }
    height: parent.height - mainAppTitle.height + mainAppTitle.radius
    visible: false

    function getUrl() {
        if (browserLoader.item)
            return browserLoader.item.url

        return ""
    }

    function hide() {
        if (visible) {
            visible = false
            mainLists.enabled = true
            return true
        }

        return false
    }

    function show(url_for_show) {
        if (!browserLoader.item)
            browserLoader.sourceComponent = browserWithProgressBar

        browserLoader.item.url = url_for_show
        mainWindow.hideAllPanels()
        mainLists.enabled = false
        visible = true
        mainAppTitle.closeButton.state = "CLOSE"
        return true
    }

    Loader {
        id: browserLoader
        anchors.fill: parent
    }

    Component {
        id: browserWithProgressBar

        ColumnLayout {
            spacing: 0
            anchors.fill: parent

            property alias url: internalBrowser.url

            ProgressBar {
                Layout.fillWidth: true
                visible: internalBrowser.loading
                from: 0
                to: 100
                value: internalBrowser.loadProgress
            }

            WebView {
                id: internalBrowser
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: true
            }
        }
    }
}
