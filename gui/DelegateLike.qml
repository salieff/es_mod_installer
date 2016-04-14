import QtQuick 2.5
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

RowLayout {
    function calculateFiveScore(lk, dslk) {
        if (lk <= 0 && dslk <= 0)
            return 0

        if (lk <= 0)
            return 1.0

        return 1.0 + lk * 4.5 / (lk + dslk)
    }

    function calculateImgSize(lk, dslk, myl) {
         if (myl !== ESModElement.LikeMarkNotFound)
             return 5

          if (lk === 0 && dslk === 0)
              return 5

          return 3
    }

    property var modeldata
    property Rectangle outrect: parent.parent
    property real fiveScore: calculateFiveScore(modeldata.likemarkscount, modeldata.dislikemarkscount)

    MMImage {
        id: likeImg
        source: {
            if (fiveScore < 1)
                "/icons/nolikes.png"
            else if (fiveScore >= 3)
                "/icons/like.png"
            else
                "/icons/dislike.png"
        }
        mmwidth: calculateImgSize(modeldata.likemarkscount, modeldata.dislikemarkscount, modeldata.mylikemark)
        mmheight: calculateImgSize(modeldata.likemarkscount, modeldata.dislikemarkscount, modeldata.mylikemark)
    }

    Text {
        id: likeText
        font.pointSize: modeldata.mylikemark === ESModElement.LikeMarkNotFound ? 12 : 16
        font.bold: modeldata.mylikemark !== ESModElement.LikeMarkNotFound
        font.italic: true
        style: Text.Raised
        // color: fiveScore >= 3 ? "lightgreen" : "lightblue"
        color: "#ff6060"
        styleColor: "red"
        text: {
            if (fiveScore < 1)
                ''
            else if (fiveScore >= 1 && fiveScore < 1.17)
                '1'
            else if (fiveScore >= 1.17 && fiveScore < 1.5)
                '1+'
            else if (fiveScore >= 1.5 && fiveScore < 1.83)
                '2-'
            else if (fiveScore >= 1.83 && fiveScore < 2.17)
                '2'
            else if (fiveScore >= 2.17 && fiveScore < 2.5)
                '2+'
            else if (fiveScore >= 2.5 && fiveScore < 2.83)
                '3-'
            else if (fiveScore >= 2.83 && fiveScore < 3.17)
                '3'
            else if (fiveScore >= 3.17 && fiveScore < 3.5)
                '3+'
            else if (fiveScore >= 3.5 && fiveScore < 3.83)
                '4-'
            else if (fiveScore >= 3.83 && fiveScore < 4.17)
                '4'
            else if (fiveScore >= 4.17 && fiveScore < 4.5)
                '4+'
            else if (fiveScore >= 4.5 && fiveScore < 4.83)
                '5-'
            else if (fiveScore >= 4.83 && fiveScore < 5.17)
                '5'
            else if (fiveScore >= 5.17)
                '5+'
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (modeldata.likemarkscount >=0 && modeldata.dislikemarkscount >= 0)
            {
                var operationModel = mainDelegateContainer.ListView.view.model
                mainWindow.likeBoxSignal(operationModel, modeldata)
            }
        }
    }
}
