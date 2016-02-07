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
            source: "/icons/info.png"
            mmwidth: 6
            mmheight: 6
            state: "NORMAL"

            states: [
                State {
                    name: "NORMAL"
                    PropertyChanges { target: button; source: "/icons/info.png"}
                },
                State {
                    name: "CLOSE"
                    PropertyChanges { target: button; source: "/icons/abort.png"}
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
