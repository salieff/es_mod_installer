import QtQuick 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: mainDelegateContainer

    property int margin: 10

    height: mainLayout.implicitHeight + margin * 2
    width: ListView.view.width
    radius: 15
    border.width: 2
    border.color: "#22000000"
    color: "darkslategrey"

    DelegateProgress {}

    RowLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: parent.width - margin * 2
        spacing: parent.margin

        DelegateLeftButton { id: leftButton }
        DelegateHeart { id: favHeart }

        DelegateTitle {
            maxTextWidth: mainLayout.width - (mainLayout.spacing * 4) - leftButton.width - favHeart.width - 10 - likeSign.width - langSign.width - (rightButton.visible ? (mainLayout.spacing + rightButton.width) : 0) - 10
        }

        DelegateLike { id: likeSign }
        DelegateLangFlags { id: langSign }
        DelegateRightButton { id: rightButton }
    }
}
