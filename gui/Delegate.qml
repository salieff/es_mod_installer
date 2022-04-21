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

        MMImage {
            id: favHeart
            source: model.favorite ? "/icons/fav.png" : "/icons/not_fav.png"
            mmwidth: 3
            mmheight: 3
            Layout.leftMargin: 10

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    let operationModel = mainDelegateContainer.ListView.view.model
                    operationModel.ToggleFavorite(model.index)
                }
            }
        }

        DelegateTitle {
            modeldata: model
            maxTextWidth: mainLayout.width - (mainLayout.spacing * 4) - leftButton.width - favHeart.width - 10 - likeSign.width - langSign.width - (rightButton.visible ? (mainLayout.spacing + rightButton.width) : 0) - 10
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
