import QtQuick 2.15
import QtQuick.Layouts 1.15


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
