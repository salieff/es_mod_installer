import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2

ApplicationWindow {
    visible: true
    width: 720
    height: 900
    title: qsTr("ES Manager")

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        ListView {
            id: mainListView
            Layout.fillWidth: true
            anchors.top: appTitle.bottom
            anchors.bottom: sortSearchBox.top
            model: esModel
            delegate: Delegate {}
            anchors.margins: 10
            spacing: 5
            // clip: true
            maximumFlickVelocity: 5000

            remove: Transition {
                NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
                NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
            }

            displaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
            }

            BusyIndicator {
                objectName: "viewBusyIndicator"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.margins: 40
                width: 200
                height: 200
                running: false;
            }

        }

        Rectangle {
            id: appTitle
            anchors.top: parent.top
            anchors.topMargin: -radius
            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(appTitleText.implicitHeight, helpImage.height) + 20 + radius
            radius: 10

            gradient: Gradient {
                GradientStop { position: 0; color: "#FFFFFF" }
                GradientStop { position: 1; color: "#A0A0A0" }
            }

            RowLayout {
                anchors {
                    fill: parent
                    topMargin: appTitle.radius
                    leftMargin: 10
                    rightMargin: 10
                }
                spacing: 10

                Text {
                    id: appTitleText
                    objectName: "appTitleText"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 18
                    wrapMode: Text.Wrap
                    style: Text.Raised
                    styleColor: "white"
                    text: qsTr("ES mod manager")
                }

                Image {
                    id: helpImage
                    source: "icons/info.png"
                    sourceSize.width: Screen.pixelDensity * 6
                    sourceSize.height: Screen.pixelDensity * 6
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    MouseArea {
                        anchors.fill: parent
                        PropertyAnimation {id:fadein; target: infoRect; property: "opacity"; from: 0; to: 0.95; duration: 300}
                        PropertyAnimation {id:fadeout; target: infoRect; property: "opacity"; from: 0.95; to: 0; duration: 300}
                        onClicked: {
                            if (infoRect.visible) {
                                fadeout.start()
                            }
                            else {
                                infoRect.visible = true
                                mainListView.enabled = false
                                fadein.start()
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: sortSearchBox
            Layout.fillWidth: true
            Layout.preferredHeight: sortSearchLayout.implicitHeight + 20 + radius
            anchors.bottom: parent.bottom
            anchors.bottomMargin: -radius
            radius: 10
            gradient: Gradient {
                GradientStop { position: 0; color: "#F0F0F0" }
                GradientStop { position: 1; color: "#909090" }
            }

            RowLayout {
                id: sortSearchLayout
                anchors {
                    fill: parent
                    bottomMargin: sortSearchBox.radius
                    leftMargin: 10
                    rightMargin: 10
                }
                spacing: 10

                Rectangle {
                    width: Screen.pixelDensity * 6
                    height: Screen.pixelDensity * 6
                    radius: 10
                    gradient: Gradient {
                        GradientStop { position: 0; color: "#FFFFFF" }
                        GradientStop { position: 1; color: "#A0A0A0" }
                    }

                    Text {
                        id: sortText
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 16
                        style: Text.Raised
                        styleColor: "white"
                        text: "•"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: sortMenu.popup()
                    }
                }

                TextField {
                    id: searchText
                    Layout.fillWidth: true
                    font.pointSize: 18
                    placeholderText: qsTr("Name filter")
                    onEditingFinished: esModel.filterByKeywords(searchText.text)
                }

                Image {
                    source: "icons/abort.png"
                    sourceSize.width: Screen.pixelDensity * 6
                    sourceSize.height: Screen.pixelDensity * 6
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            searchText.text = ""
                            esModel.filterByKeywords(searchText.text)
                        }
                    }
                }

                Image {
                    source: "icons/search.png"
                    sourceSize.width: Screen.pixelDensity * 6
                    sourceSize.height: Screen.pixelDensity * 6
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height

                    MouseArea {
                        anchors.fill: parent
                        onClicked: esModel.filterByKeywords(searchText.text)
                    }

                }
            }
        }
    }

    Rectangle {
        id: infoRect
        height: parent.height - appTitle.height
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 5
        }
        radius: 10
        opacity: 0
        visible: false
        clip: true
        onOpacityChanged: if (opacity == 0) {
                              visible = false
                              mainListView.enabled = true
                          }

        Flickable {
            anchors.fill: parent;
            anchors.margins: 10
            contentHeight: helpText.implicitHeight

            Text {
                id: helpText
                objectName: "helpText"
                width: parent.width
                font.pointSize: 18
                wrapMode: Text.Wrap
                text: qsTr("<h3>Hello!</h3><br><p>If you see this text instead of help so you don't have internet connection to download help topic!</p>")
            }
        }
    }

    Menu {
        id: sortMenu
        title: qsTr("Sort mode")

        MenuItem {
            text: qsTr("As server")
            onTriggered: {
                esModel.sortAsServer()
                sortText.text = "•"
            }
        }

        MenuItem {
            text: qsTr("By name a→Z")
            onTriggered: {
                esModel.sortByName(0)
                sortText.text = "aZ↑"
            }
        }

        MenuItem {
            text: qsTr("By name Z→a")
            onTriggered: {
                esModel.sortByName(1)
                sortText.text = "Za↓"
            }
        }

        MenuItem {
            text: qsTr("By size 1→99")
            onTriggered: {
                esModel.sortBySize(0)
                sortText.text = "Sz↑"
            }
        }

        MenuItem {
            text: qsTr("By size 99→1")
            onTriggered: {
                esModel.sortBySize(1)
                sortText.text = "Sz↓"
            }
        }

        MenuItem {
            text: qsTr("By date 1.1.1970 → 12.12.2015")
            onTriggered: {
                esModel.sortByDate(0)
                sortText.text = "Dt↑"
            }
        }

        MenuItem {
            text: qsTr("By date 12.12.2015 → 1.1.1970")
            onTriggered: {
                esModel.sortByDate(1)
                sortText.text = "Dt↓"
            }
        }
    }
}
