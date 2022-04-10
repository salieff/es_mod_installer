import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: mainDelegateContainer

    property int margin: 10

    anchors.horizontalCenter: parent.horizontalCenter
    height: mainLayout.implicitHeight + margin * 2
    width: parent.width
    radius: 15
    border.width: 2
    border.color: "#22000000"
    color: "darkslategrey"

    DelegateProgress {
        modeldata: model
    }

    RowLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: parent.width - margin * 2
        spacing: parent.margin

        DelegateLeftButton {
            id: leftButton
            modeldata: model
        }

        DelegateTitle {
            modeldata: model
            maxTextWidth: mainLayout.width - (mainLayout.spacing * 3) - leftButton.width - likeSign.width - langSign.width - (rightButton.visible ? (mainLayout.spacing + rightButton.width) : 0)
        }

        DelegateLike {
            id: likeSign
            modeldata: model
        }

        DelegateLangFlags {
            id: langSign
            modeldata: model
        }

        DelegateRightButton {
            id: rightButton
            modeldata: model
        }
    }
}
