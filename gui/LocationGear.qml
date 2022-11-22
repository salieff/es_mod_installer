import QtQuick 2.15


MMImage {
    anchors {
        top: parent.top
        right: parent.right
        topMargin: 5
        rightMargin: 20
    }

    source: "/icons/gear.png"
    mmwidth: 5
    mmheight: 5
    opacity: 0.75

    MouseArea {
        anchors.fill: parent
        onClicked: mainInstallLocationPanel.show()
    }
}
