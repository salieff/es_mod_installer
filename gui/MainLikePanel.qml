import QtQuick 2.3
import QtQuick.Layouts 1.2
import org.salieff.esmodinstaller 1.0

Rectangle {
    property ListView view
    property var self

    width: likeRectLayout.implicitWidth + 40
    height: likeRectLayout.implicitHeight + 40
    anchors.centerIn: parent
    radius: 10
    color: "#e0ffffff"
    opacity: 0
    visible: false

    Behavior on opacity { NumberAnimation { duration: 300 } }

    property int itemIndex: -1
    property int myLike: ESModElement.LikeMarkNotFound

    onOpacityChanged: if (opacity == 0) {
                          visible = false
                          view.enabled = true
                      }

    function hide() {
        if (visible) {
            opacity = 0
            return true
        }
        return false
    }

    function show() {
        if (visible)
            return false

        view.enabled = false
        visible = true
        opacity = 0.95
        return true
    }

    ColumnLayout {
        id: likeRectLayout
        anchors.centerIn: parent
        spacing: 20

        Text {
            id: likeTitle
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            font.pointSize: 18
            color: "#606060"
            text: qsTr("ES mod title")
        }

        MMImage {
            id: likeImg
            source: "/icons/like.png"
            mmwidth: 20
            mmheight: 20
            opacity: 1

            MouseArea {
                anchors.fill: parent

                onPressed: {
                    if (self.myLike !== ESModElement.LikeMark)
                    {
                        likeImg.opacity = 1
                        dislikeImg.opacity = 0.3
                    }
                }

                onReleased: {
                    if (self.myLike !== ESModElement.LikeMark)
                    {
                        self.myLike = ESModElement.LikeMark
                        esModel.SendLike(itemIndex, ESModElement.LikeMark)
                        self.hide()
                    }
                }
            }

        }

        MMImage {
            id: dislikeImg
            source: "/icons/dislike.png"
            mmwidth: 20
            mmheight: 20
            opacity: 0.3

            MouseArea {
                anchors.fill: parent

                onPressed: {
                    if (self.myLike !== ESModElement.DislikeMark)
                    {
                        likeImg.opacity = 0.3
                        dislikeImg.opacity = 1
                    }
                }

                onReleased: {
                    if (self.myLike !== ESModElement.DislikeMark)
                    {
                        self.myLike = ESModElement.DislikeMark
                        esModel.SendLike(itemIndex, ESModElement.DislikeMark)
                        self.hide()
                    }
                }
            }
        }

        RowLayout {
            id: likeMiniRow
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            MMImage {
                id: likeMiniImg
                source: "/icons/like.png"
                mmwidth: 6
                mmheight: 6
            }

            Text {
                id: likeText
                font.pointSize: 24
                style: Text.Sunken
                color: "lightgreen"
                styleColor: "black"
                text: "33"
            }

            MMImage {
                id: dislikeMiniImg
                source: "/icons/dislike.png"
                mmwidth: 6
                mmheight: 6
            }

            Text {
                id: dislikeText
                font.pointSize: 24
                style: Text.Sunken
                color: "lightblue"
                styleColor: "black"
                text: "33"
            }

        }
    }

    Connections {
        target: view
        onLikeBoxSignal: {
            if (!visible)
            {
                if (modeldata.mylikemark === ESModElement.LikeMark)
                    likeImg.opacity = 1
                else
                    likeImg.opacity = 0.3

                if (modeldata.mylikemark === ESModElement.DislikeMark)
                    dislikeImg.opacity = 1
                else
                    dislikeImg.opacity = 0.3

                if (modeldata.likemarkscount > 0)
                {
                    likeText.text = modeldata.likemarkscount
                    likeMiniImg.visible = true;
                    likeText.visible = true;
                }
                else
                {
                    likeMiniImg.visible = false;
                    likeText.visible = false;
                }

                if (modeldata.dislikemarkscount > 0)
                {
                    dislikeText.text = modeldata.dislikemarkscount
                    dislikeMiniImg.visible = true;
                    dislikeText.visible = true;
                }
                else
                {
                    dislikeMiniImg.visible = false;
                    dislikeText.visible = false;
                }

                if (modeldata.likemarkscount <= 0 && modeldata.dislikemarkscount <= 0)
                    likeMiniRow.visible = false
                else
                    likeMiniRow.visible = true

                view.enabled = false

                var tlen = modeldata.title.length
                var maxlen = 25
                if ( tlen <= maxlen)
                    likeTitle.text = modeldata.title
                else
                    likeTitle.text = modeldata.title.substring(0, (maxlen - 3) / 2) + "..." + modeldata.title.substring(tlen - (maxlen - 3) / 2)

                self.itemIndex = modeldata.index
                self.myLike = modeldata.mylikemark
                show()
            }
        }
    }
}
