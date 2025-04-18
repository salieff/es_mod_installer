import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

Rectangle {
    property alias closeButton: mailCloseButton

    Layout.topMargin: -radius
    Layout.fillWidth: true
    Layout.preferredHeight: appTitleLayout.implicitHeight + 20 + radius
    radius: 10

    gradient: Gradient {
        GradientStop { position: 0; color: "#FFFFFF" }
        GradientStop { position: 1; color: "#A0A0A0" }
    }

    RowLayout {
        id: appTitleLayout
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: parent.radius + 10
        spacing: 10

        MMImage {
            id: helpButton
            source: "/icons/help.png"
            mmwidth: 6
            mmheight: 6

            function showHelp(chapter) {
                if (Qt.platform.os === "android")
                    mainWebView.show("file:///android_asset/help/index.html" + chapter)
                else if (Qt.platform.os === "ios")
                    mainWebView.show("file:///Applications/ESManager.app/help.html" + chapter)
            }

            MouseArea {
                anchors.fill: parent
                onClicked: helpButton.showHelp("")
            }

            Connections {
                target: esModel
                function onShowMeHelp(chapter) { helpButton.showHelp(chapter) }
            }
        }

        Text {
            id: appTitleText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 18
            wrapMode: Text.Wrap
            style: Text.Raised
            styleColor: "white"
            text: qsTr("ES mod manager")

            Connections {
                target: esModel
                function onAppTitleReceived(text) { appTitleText.text = text }
            }

            MainMenu { id: mainMenu }

            MouseArea {
                anchors.fill: parent
                onPressAndHold: mainMenu.popup()
                onContainsPressChanged: {
                    if (containsPress) {
                        appTitleText.color = "blue"
                    }
                    else {
                        appTitleText.color = "black"
                    }
                }
            }
        }

        MMImage {
            id: externalLinkButton
            source: "/icons/external-link.png"
            visible: mainWebView.visible && !mainWebView.getUrl().toString().startsWith("file:/")
            mmwidth: 6
            mmheight: 6

            MouseArea {
                anchors.fill: parent
                onClicked: Qt.openUrlExternally(mainWebView.getUrl())
            }
        }

        MMImage {
            id: mailCloseButton
            source: "/icons/mail.png"
            mmwidth: 6
            mmheight: 6
            property bool pulsed: false
            state: "NORMAL"

            SequentialAnimation on opacity {
                id: opacityAnim
                running: false
                loops: Animation.Infinite
                NumberAnimation { from: 1; to: 0.25; duration: 500; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.25; to: 1; duration: 500; easing.type: Easing.InOutQuad }
            }

            states: [
                State {
                    name: "NORMAL"
                    PropertyChanges { target: mailCloseButton; source: "/icons/mail.png"}
                    PropertyChanges { target: mailCloseButton; opacity: 1}
                    PropertyChanges { target: opacityAnim; running: mailCloseButton.pulsed}
                },
                State {
                    name: "CLOSE"
                    PropertyChanges { target: mailCloseButton; opacity: 1}
                    PropertyChanges { target: opacityAnim; running: false}
                    PropertyChanges { target: mailCloseButton; source: "/icons/abort.png"}
                }
            ]

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (parent.state == "CLOSE")
                    {
                        mainWindow.hideAllPanels()
                        parent.state = "NORMAL"
                    }
                    else
                    {
                        // mainWindow.hideAllPanels()
                        mainInfoPanel.show()
                    }
                }
            }
        }
    }
}
