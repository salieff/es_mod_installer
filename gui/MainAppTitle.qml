import QtQuick 2.3
import QtQuick.Layouts 1.2

Rectangle {
    property MainInfoPanel infoRect
    property MainWebView webView
    property MainLikePanel likePanel

    property MMImage button: iButton

    anchors {
        top: parent.top
        topMargin: -radius
        left: parent.left
        right: parent.right
    }
    height: appTitleLayout.implicitHeight + 20 + radius
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

            MouseArea {
                anchors.fill: parent
                onClicked: mainListView.infoUriSignal("file:///android_asset/help/index.html")
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
                onAppTitleReceived: appTitleText.text = text
            }
        }

        MMImage {
            id: iButton
            source: "/icons/mail.png"
            mmwidth: 6
            mmheight: 6
            // state: "NORMAL"
            state: "PULSE"

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
                    PropertyChanges { target: iButton; source: "/icons/mail.png"}
                    PropertyChanges { target: iButton; opacity: 1}
                    PropertyChanges { target: opacityAnim; running: false}
                },
                State {
                    name: "CLOSE"
                    PropertyChanges { target: iButton; opacity: 1}
                    PropertyChanges { target: opacityAnim; running: false}
                    PropertyChanges { target: button; source: "/icons/abort.png"}
                },
                State {
                    name: "PULSE"
                    PropertyChanges { target: opacityAnim; running: true}
                }
            ]

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (parent.state == "CLOSE")
                    {
                        infoRect.hide()
                        webView.hide()
                        likePanel.hide()
                        parent.state = "NORMAL"
                    }
                    else
                    {
                        webView.hide()
                        likePanel.hide()
                        infoRect.show()
                    }
                }
            }
        }
    }
}
