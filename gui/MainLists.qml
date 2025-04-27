import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Window


Item {
    Layout.fillHeight: true
    Layout.fillWidth: true

    SwipeView {
        id: swipeView
        anchors.fill: parent
        anchors.margins: mm(1)
        spacing: mm(1)
        currentIndex: 2

        MainListView {
            model: esBrokenModel
            headerText: qsTr("Broken")
        }

        MainListView {
            model: esFavoriteModel
            headerText: qsTr("Favorite")
        }

        MainListView {
            model: esModel
            headerText: qsTr("All")
        }

        MainListView {
            model: esInstalledModel
            headerText: qsTr("Installed")
        }

        MainListView {
            model: esIncompletedModel
            headerText: qsTr("Incomplete")
        }
    }

    Rectangle {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        width: pageIndicator.implicitWidth + mm(1)
        height: pageIndicator.implicitHeight + mm(1)
        z: 1
        radius: mm(1)

        gradient: Gradient {
            GradientStop { position: 0; color: "#FFFFFF" }
            GradientStop { position: 1; color: "#A0A0A0" }
        }
        opacity: 0.9

        PageIndicator {
            id: pageIndicator
            anchors.centerIn: parent
            spacing: mm(0.5)
            padding: mm(0.5)

            count: swipeView.count
            currentIndex: swipeView.currentIndex

            delegate: Rectangle {
                implicitWidth: mm(1)
                implicitHeight: mm(1)

                radius: width / 2
                color: "black"

                opacity: index === pageIndicator.currentIndex ? 0.95 : pressed ? 0.7 : 0.45

                Behavior on opacity {
                    OpacityAnimator {
                        duration: 100
                    }
                }
            }
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        width: mm(20)
        height: mm(20)
        running: true

        Connections {
            target: esModel
            function onEsIndexReceived() {
                busyIndicator.running = false
                busyIndicator.visible = false
            }
        }
    }
}
