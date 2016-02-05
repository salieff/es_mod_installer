import QtQuick 2.5
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

RowLayout {
    property var modeldata
    property Rectangle outrect: parent.parent

    MMImage {
        id: likeImg
        source: modeldata.index % 2 != 0 ? "/icons/like.png" : "/icons/dislike.png"
        mmwidth: 3
        mmheight: 3
    }

    Text {
        id: likeText
        font.pointSize: 12
        style: Text.Sunken
        color: modeldata.index % 2 != 0 ? "lightgreen" : "lightblue"
        styleColor: "black"
        text: 33 + (modeldata.index % 2) * 10
    }

    MouseArea {
        anchors.fill: parent

        // signal likeBoxSignal(int itemIndex, int myLike, bool myDislike, int likes, int dislikes)
        onClicked: {
            var myMark = ESModElement.LikeMark

            if (modeldata.index % 3 === 1)
                myMark = ESModElement.DislikeMark

            if (modeldata.index % 3 === 2)
                myMark = ESModElement.LikeMarkNotFound

            outrect.ListView.view.likeBoxSignal(modeldata.index, myMark, 23 + (modeldata.index % 2), 36 - (modeldata.index % 2))
        }
    }
}
