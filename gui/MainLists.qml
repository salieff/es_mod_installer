import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0
import QtQuick.Window 2.2

Item {
    Layout.fillHeight: true
    Layout.fillWidth: true

    property int lastIndex: 1

    Flickable {
        anchors.fill: parent

        flickableDirection: Flickable.HorizontalFlick
        flickDeceleration: 999999999

        contentWidth: listsLayout.implicitWidth
        contentX: listsLayout.implicitWidth / 3

        RowLayout {
            id: listsLayout
            spacing: 0
            anchors.centerIn: parent
            height: parent.height

            MainListView {
                model: esServerModel
                headerText: qsTr("Not Installed")
            }

            MainListView {
                model: esModel
                headerText: qsTr("All")
            }

            MainListView {
                model: esInstalledModel
                headerText: qsTr("Installed")
            }
        }

        Behavior on contentX {
            NumberAnimation {
                duration: 300
                easing.type: Easing.OutBack
            }
        }

        onMovementEnded: {
            lastIndex = Math.round(contentX / mainWindow.width)
            contentX = mainWindow.width * lastIndex
        }

        onContentXChanged: {
            var ind2 = Math.round(contentX / mainWindow.width)
            pageIndicator.currentIndex = ind2
        }

        onContentWidthChanged: {
            contentX = mainWindow.width * lastIndex
            pageIndicator.currentIndex = lastIndex
        }
    }

    Rectangle {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        width: pageIndicator.implicitWidth + 20
        height: pageIndicator.implicitHeight + 20
        z: 1
        radius: 10

        gradient: Gradient {
            GradientStop { position: 0; color: "#FFFFFF" }
            GradientStop { position: 1; color: "#A0A0A0" }
        }
        opacity: 0.9

        PageIndicator {
            id: pageIndicator
            anchors.centerIn: parent

            count: 3
            currentIndex: 1
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        width: 200
        height: 200
        running: true;

        Connections {
            target: esModel
            onEsIndexReceived: busyIndicator.running = false;
        }
    }
}
