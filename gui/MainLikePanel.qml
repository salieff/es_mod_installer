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

    function fillData(operationModel, element) {
        if (!visible)
        {
            likeImg.opacity = element.mylikemark === ESModElement.LikeMark ? 1 : 0.3
            dislikeImg.opacity = element.mylikemark === ESModElement.DislikeMark ? 1 : 0.3

            likeText.text = element.likemarkscount > 0 ? element.likemarkscount : ""
            dislikeText.text = element.dislikemarkscount > 0 ? element.dislikemarkscount : ""

            mainLists.enabled = false

            let tlen = element.title.length
            let maxlen = 32
            if ( tlen <= maxlen)
                likeTitle.text = element.title
            else
                likeTitle.text = element.title.substring(0, (maxlen - 3) / 2) + "..." + element.title.substring(tlen - (maxlen - 3) / 2)

            instTotal.text = element.insttotal > 0 ? element.insttotal : ""
            instActive.text = element.instactive > 0 ? element.instactive : ""
            instTotalMonth.text = element.insttotalmonth > 0 ? element.insttotalmonth : ""
            instActiveMonth.text = element.instactivemonth > 0 ? element.instactivemonth : ""
            instTotalWeek.text = element.insttotalweek > 0 ? element.insttotalweek : ""
            instActiveWeek.text = element.instactiveweek > 0 ? element.instactiveweek : ""
            lifeTimeAVG.text = element.lifetimeavg > 0 ? element.lifetimeavg : ""
            lifeTimeMAX.text = element.lifetimemax > 0 ? element.lifetimemax : ""

            itemIndex = element.index
            myLike = element.mylikemark
            opModel = operationModel
            show()
        }
    }
}
