import QtQuick 2.5
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

RowLayout {
    property var modeldata
    property Rectangle outrect: parent.parent

    MMImage {
        id: likeImg
        source: {
            if (modeldata.likemarkscount === 0 && modeldata.dislikemarkscount === 0)
                source = "/icons/nolikes.png"
            else if (likeText.fiveScore >= 3)
                source = "/icons/like.png"
            else
                source = "/icons/dislike.png"
        }
        mmwidth: modeldata.mylikemark !== ESModElement.LikeMarkNotFound || (modeldata.likemarkscount === 0 && modeldata.dislikemarkscount === 0) ? 5 : 3
        mmheight: modeldata.mylikemark !== ESModElement.LikeMarkNotFound || (modeldata.likemarkscount === 0 && modeldata.dislikemarkscount === 0) ? 5 : 3
    }

    Text {
        property real fiveScore: modeldata.likemarkscount === 0 ? 1.0 : 1.0 + modeldata.likemarkscount * 4.5 / (modeldata.likemarkscount + modeldata.dislikemarkscount)

        id: likeText
        visible: modeldata.likemarkscount > 0 || modeldata.dislikemarkscount > 0
        font.pointSize: modeldata.mylikemark === ESModElement.LikeMarkNotFound ? 12 : 16
        font.bold: modeldata.mylikemark !== ESModElement.LikeMarkNotFound
        style: Text.Sunken
        color: likeText.fiveScore >= 3 ? "lightgreen" : "lightblue"
        styleColor: "black"
        text: {
            if (likeText.fiveScore < 1.17)
                text = '1'
            else if (likeText.fiveScore >= 1.17 && likeText.fiveScore < 1.5)
                text = '1+'
            else if (likeText.fiveScore >= 1.5 && likeText.fiveScore < 1.83)
                text = '2-'
            else if (likeText.fiveScore >= 1.83 && likeText.fiveScore < 2.17)
                text = '2'
            else if (likeText.fiveScore >= 2.17 && likeText.fiveScore < 2.5)
                text = '2+'
            else if (likeText.fiveScore >= 2.5 && likeText.fiveScore < 2.83)
                text = '3-'
            else if (likeText.fiveScore >= 2.83 && likeText.fiveScore < 3.17)
                text = '3'
            else if (likeText.fiveScore >= 3.17 && likeText.fiveScore < 3.5)
                text = '3+'
            else if (likeText.fiveScore >= 3.5 && likeText.fiveScore < 3.83)
                text = '4-'
            else if (likeText.fiveScore >= 3.83 && likeText.fiveScore < 4.17)
                text = '4'
            else if (likeText.fiveScore >= 4.17 && likeText.fiveScore < 4.5)
                text = '4+'
            else if (likeText.fiveScore >= 4.5 && likeText.fiveScore < 4.83)
                text = '5-'
            else if (likeText.fiveScore >= 4.83 && likeText.fiveScore < 5.17)
                text = '5'
            else if (likeText.fiveScore >= 5.17)
                text = '5+'
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: outrect.ListView.view.likeBoxSignal(modeldata)
    }
}
