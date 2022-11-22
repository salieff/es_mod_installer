import QtQuick 2.15
import QtQuick.Layouts 1.15


MMImage {
    source: model.favorite ? "/icons/fav.png" : "/icons/not_fav.png"
    mmwidth: 4
    mmheight: 4
    Layout.leftMargin: mm(1)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            let operationModel = mainDelegateContainer.ListView.view.model
            operationModel.ToggleFavorite(model.index)
        }
    }
}
