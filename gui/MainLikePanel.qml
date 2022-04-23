import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0

Rectangle {
    width: likeRectLayout.implicitWidth + 40
    height: likeRectLayout.implicitHeight + 40
    anchors.centerIn: parent
    radius: 10
    color: "#f0ffffff"
    opacity: 0
    visible: false

    Behavior on opacity { NumberAnimation { duration: 300 } }

    property int itemIndex: -1
    property int myLike: ESModElement.LikeMarkNotFound
    property var opModel
    onOpacityChanged: if (opacity == 0) {
                          visible = false
                          mainLists.enabled = true
                      }

    function hide() {
        if (visible) {
            opacity = 0
            return true
        }
        return false
    }

    function show() {
        // if (visible)
        //    return false

        mainWindow.hideAllPanels()

        mainLists.enabled = false
        visible = true
        opacity = 1
        mainAppTitle.closeButton.state = "CLOSE"
        return true
    }

    ColumnLayout {
        id: likeRectLayout
        anchors.centerIn: parent
        spacing: 20

        Text {
            id: likeTitle
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            font.pointSize: 18
            style: Text.Raised
            styleColor: "white"
            text: qsTr("ES mod title")
        }

        GridLayout {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            columnSpacing: 20
            rowSpacing: 20
            columns: 2

            MMImage {
                id: likeImg
                Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                source: "/icons/like.png"
                mmwidth: 13
                mmheight: 13
                opacity: 1

                MouseArea {
                    anchors.fill: parent

                    onPressed: {
                        if (myLike !== ESModElement.LikeMark)
                        {
                            likeImg.opacity = 1
                            dislikeImg.opacity = 0.3
                        }
                    }

                    onReleased: {
                        if (myLike !== ESModElement.LikeMark)
                        {
                            myLike = ESModElement.LikeMark
                            opModel.SendLike(itemIndex, ESModElement.LikeMark)
                            mainAppTitle.closeButton.state = "NORMAL"
                            hide()
                        }
                    }
                }

            }

            Text {
                id: likeText
                Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                font.pointSize: 24
                style: Text.Sunken
                color: "lightgreen"
                styleColor: "black"
                text: "33"
            }

            MMImage {
                id: dislikeImg
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                source: "/icons/dislike.png"
                mmwidth: 13
                mmheight: 13
                opacity: 0.3

                MouseArea {
                    anchors.fill: parent

                    onPressed: {
                        if (myLike !== ESModElement.DislikeMark)
                        {
                            likeImg.opacity = 0.3
                            dislikeImg.opacity = 1
                        }
                    }

                    onReleased: {
                        if (myLike !== ESModElement.DislikeMark)
                        {
                            myLike = ESModElement.DislikeMark
                            opModel.SendLike(itemIndex, ESModElement.DislikeMark)
                            mainAppTitle.closeButton.state = "NORMAL"
                            hide()
                        }
                    }
                }
            }

            Text {
                id: dislikeText
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                font.pointSize: 24
                style: Text.Sunken
                color: "lightblue"
                styleColor: "black"
                text: "33"
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            spacing: 40

            // Installation statistics block
            GridLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                columns: 3

                Text {
                    font.pointSize: 18
                    style: Text.Raised
                    styleColor: "white"
                    text: qsTr("Installs")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("Active")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("Total")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("Last week")
                }

                Text {
                    id: instActiveWeek
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightgreen"
                    styleColor: "black"
                    text: qsTr("111")
                }

                Text {
                    id: instTotalWeek
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightblue"
                    styleColor: "black"
                    text: qsTr("222")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("Last month")
                }

                Text {
                    id: instActiveMonth
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightgreen"
                    styleColor: "black"
                    text: qsTr("333")
                }

                Text {
                    id: instTotalMonth
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightblue"
                    styleColor: "black"
                    text: qsTr("444")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("All period")
                }

                Text {
                    id: instActive
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightgreen"
                    styleColor: "black"
                    text: qsTr("555")
                }

                Text {
                    id: instTotal
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightblue"
                    styleColor: "black"
                    text: qsTr("666")
                }
            }

            // Life time block
            GridLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                columns: 2

                Text {
                    Layout.columnSpan: 2
                    font.pointSize: 18
                    style: Text.Raised
                    styleColor: "white"
                    text: qsTr("Hours of life")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("Average")
                }

                Text {
                    id: lifeTimeAVG
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightgreen"
                    styleColor: "black"
                    text: qsTr("111")
                }

                Text {
                    font.pointSize: 14
                    color: "#606060"
                    text: qsTr("Maximum")
                }

                Text {
                    id: lifeTimeMAX
                    font.pointSize: 18
                    style: Text.Sunken
                    color: "lightblue"
                    styleColor: "black"
                    text: qsTr("444")
                }
            }
        }
    }

    Connections {
        target: mainWindow
        function onLikeBoxSignal(operationModel, model) {
            if (!visible)
            {
                likeImg.opacity = model.mylikemark === ESModElement.LikeMark ? 1 : 0.3
                dislikeImg.opacity = model.mylikemark === ESModElement.DislikeMark ? 1 : 0.3

                likeText.text = model.likemarkscount > 0 ? model.likemarkscount : ""
                dislikeText.text = model.dislikemarkscount > 0 ? model.dislikemarkscount : ""

                mainLists.enabled = false

                let tlen = model.title.length
                let maxlen = 32
                if ( tlen <= maxlen)
                    likeTitle.text = model.title
                else
                    likeTitle.text = model.title.substring(0, (maxlen - 3) / 2) + "..." + model.title.substring(tlen - (maxlen - 3) / 2)

                instTotal.text = model.insttotal > 0 ? model.insttotal : ""
                instActive.text = model.instactive > 0 ? model.instactive : ""
                instTotalMonth.text = model.insttotalmonth > 0 ? model.insttotalmonth : ""
                instActiveMonth.text = model.instactivemonth > 0 ? model.instactivemonth : ""
                instTotalWeek.text = model.insttotalweek > 0 ? model.insttotalweek : ""
                instActiveWeek.text = model.instactiveweek > 0 ? model.instactiveweek : ""
                lifeTimeAVG.text = model.lifetimeavg > 0 ? model.lifetimeavg : ""
                lifeTimeMAX.text = model.lifetimemax > 0 ? model.lifetimemax : ""

                itemIndex = model.index
                myLike = model.mylikemark
                opModel = operationModel
                show()
            }
        }
    }
}
