import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0


Item {
    Layout.preferredWidth: likeSignLayout.implicitWidth
    Layout.preferredHeight: likeSignLayout.implicitHeight

    property var scoreStrings: ["1", "1+", "2-", "2", "2+", "3-", "3", "3+", "4-", "4", "4+", "5-", "5", "5+"]
    property real scoreIndex: calculateScoreIndex(model.likemarkscount, model.dislikemarkscount)

    function calculateScoreIndex(lk, dslk) {
        if (lk <= 0 && dslk <= 0)
            return -1

        let div1 = 0;
        let div2 = 0;

        if (lk > 0) {
            div1 = lk;
            div2 = lk;
        }

        if (dslk > 0)
            div2 += dslk;

        return Math.floor(scoreStrings.length * div1 / (div2 + 1)); // [0, arrSize)
    }

    function calculateImgSize(lk, dslk, myl) {
        if (myl !== ESModElement.LikeMarkNotFound)
            return 5

        if (lk === 0 && dslk === 0)
            return 5

        return 3
    }

    RowLayout {
        id: likeSignLayout

        MMImage {
            id: likeImg
            source: {
                if (scoreIndex < 0)
                    "/icons/nolikes.png"
                else if (scoreIndex >= 6)
                    "/icons/like.png"
                else
                    "/icons/dislike.png"
            }
            mmwidth: calculateImgSize(model.likemarkscount, model.dislikemarkscount, model.mylikemark)
            mmheight: calculateImgSize(model.likemarkscount, model.dislikemarkscount, model.mylikemark)
        }

        Text {
            id: likeText
            font.pointSize: model.mylikemark === ESModElement.LikeMarkNotFound ? 12 : 16
            font.bold: model.mylikemark !== ESModElement.LikeMarkNotFound
            font.italic: true
            style: Text.Raised
            // color: scoreIndex >= 3 ? "lightgreen" : "lightblue"
            color: "#ff6060"
            styleColor: "red"
            text: {
                if (scoreIndex < 0)
                    ''
                else
                    scoreStrings[scoreIndex]
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (model.likemarkscount >=0 && model.dislikemarkscount >= 0) {
                let operationModel = mainDelegateContainer.ListView.view.model
                mainLikePanel.fillData(operationModel, model)
            }
        }
    }
}
