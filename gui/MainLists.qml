import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Window 2.2
import org.salieff.esmodinstaller 1.0

ESSwipeArea {
    anchors {
        top: mainAppTitle.bottom
        bottom: mainSortSearchBox.top
        left: parent.left
        right: parent.right
    }

    pixelage: Screen.pixelDensity * 50
    speed: Screen.pixelDensity * 10

    state: "ALL"

    states: [
        State {
            name: "SERVER"
            AnchorChanges { target: mainListViewServer; anchors.left: parent.left; anchors.right: parent.right;}
            AnchorChanges { target: mainListViewAll; anchors.left: parent.right; anchors.right: undefined;}
            AnchorChanges { target: mainListViewLocal; anchors.left: mainListViewAll.right; anchors.right: undefined;}
        },
        State {
            name: "ALL"
            AnchorChanges { target: mainListViewServer; anchors.left: undefined; anchors.right: parent.left;}
            AnchorChanges { target: mainListViewAll; anchors.left: parent.left; anchors.right: parent.right;}
            AnchorChanges { target: mainListViewLocal; anchors.left: parent.right; anchors.right: undefined;}
        },
        State {
            name: "LOCAL"
            AnchorChanges { target: mainListViewServer; anchors.left: undefined; anchors.right: mainListViewAll.left;}
            AnchorChanges { target: mainListViewAll; anchors.left: undefined; anchors.right: parent.left;}
            AnchorChanges { target: mainListViewLocal; anchors.left: parent.left; anchors.right: parent.right;}
        }
    ]

    transitions: Transition {
        AnchorAnimation { duration: 300 }
    }

    MainListView {
        id: mainListViewServer
        model: esServerModel
        anchors.right: parent.left
        headerText: qsTr("Not Installed →")
    }

    MainListView {
        id: mainListViewAll
        model: esModel
        anchors.left: parent.left
        headerText: qsTr("← All →")
    }

    MainListView {
        id: mainListViewLocal
        model: esInstalledModel
        anchors.left: parent.right
        headerText: qsTr("← Installed")
    }

    /*
    MouseArea {
        anchors.fill: parent

        property int lastX: -1

        onPressed: {
            if (mouse.x > 20 && mouse.x < (width - 20)) {
                mouse.accepted = false
                return
            }

            lastX = mouse.x
        }

        onReleased: {
            lastX = -1;
        }

        onMouseXChanged: {
            if (lastX < 0) {
                return
            }

            var dx = mouseX - lastX

            if (Math.abs(dx) < (parent.width / 5)) {
                return
            }

            if (parent.state === "SERVER" && dx < 0) {
                parent.state = "ALL"
            }
            else if (parent.state === "ALL" && dx < 0) {
                parent.state = "LOCAL"
            }
            else if (parent.state === "LOCAL" && dx > 0) {
                parent.state = "ALL"
            }
            else if (parent.state === "ALL" && dx > 0) {
                parent.state = "SERVER"
            }

            lastX = -1;
        }
    }
    */

    onSwipeLeft: {
        if (state === "SERVER") {
            state = "ALL"
        }
        else if (state === "ALL") {
            state = "LOCAL"
        }
    }

    onSwipeRight: {
        if (state === "LOCAL") {
            state = "ALL"
        }
        else if (state === "ALL") {
            state = "SERVER"
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
