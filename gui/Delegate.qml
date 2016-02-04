import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import org.salieff.esmodinstaller 1.0

Rectangle {
    id: mainRectangle
    anchors.horizontalCenter: parent.horizontalCenter
    height: layout1.implicitHeight
    width: parent.width - margin * 2
    radius: 15
    border.width: 2
    border.color: "#22000000"
    color: "darkslategrey"

    property int margin: 10

    DelegateProgress {
        anchors.fill: parent

        percent: model.progress
        modstate: model.modstate
        borderwidth: parent.border.width
        radius: parent.radius
    }

    RowLayout {
        id: layout1
        anchors.centerIn: parent
        width: parent.width - margin * 2
        spacing: parent.margin

        DelegateLeftButton {
            id: leftButton
            modstate: model.modstate
            guiblocked: model.guiblocked
        }

        Item {
            id: delegateTextBox
            Layout.fillWidth: true
            Layout.preferredHeight: layout2.implicitHeight + mainRectangle.margin * 2

            ColumnLayout {
                id: layout2
                //anchors.centerIn: parent
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - margin * 2
                spacing: mainRectangle.margin

                Text {
                    id: delegateText1
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 13.5
                    wrapMode: Text.Wrap
                    style: Text.Sunken
                    color: "white"
                    styleColor: "black"
                    text: title
                }
                Text {
                    id: delegateText2
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: 9
                    wrapMode: Text.Wrap
                    style: Text.Sunken
                    color: "white"
                    styleColor: "black"
                    text: "[" + status + "] {" + getReadableFileSizeString(modsize) + ", " + timestamp + "}"

                    function getReadableFileSizeString(fileSizeInBytes) {
                        var i = -1;
                        var byteUnits = [' kb', ' Mb', ' Gb', ' Tb', 'Pb', 'Eb', 'Zb', 'Yb'];
                        do {
                            fileSizeInBytes = fileSizeInBytes / 1024;
                            i++;
                        } while (fileSizeInBytes > 1024 && i < (byteUnits.length - 1));

                        return Math.max(fileSizeInBytes, 0.1).toFixed(1) + byteUnits[i];
                    }

                }
            }

            MouseArea {
                anchors.fill: parent

                onContainsPressChanged: {
                    if (containsPress) {
                        delegateText1.color = "red"
                        delegateText2.color = "red"
                    }
                    else {
                        delegateText1.color = "white"
                        delegateText2.color = "white"
                    }
                }

                onClicked:  mainRectangle.ListView.view.infoUriSignal(infouri)
            }
        }

        Item {
            id: likeBox
            Layout.preferredHeight: likeLayout.implicitHeight
            Layout.preferredWidth: likeLayout.implicitWidth

            RowLayout {
                id: likeLayout

                Image {
                    id: likeImg
                    source: index % 2 != 0 ? "/icons/like.png" : "/icons/dislike.png"
                    sourceSize.width: Screen.pixelDensity * 3
                    sourceSize.height: Screen.pixelDensity * 3
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                }

                Text {
                    id: likeText
                    font.pointSize: 12
                    style: Text.Sunken
                    color: index % 2 != 0 ? "lightgreen" : "lightblue"
                    styleColor: "black"
                    text: 33 + (index % 2) * 10
                }
            }

            MouseArea {
                anchors.fill: parent

                // signal likeBoxSignal(int itemIndex, int myLike, bool myDislike, int likes, int dislikes)
                onClicked: {
                    var myMark = ESModElement.LikeMark

                    if (index % 3 === 1)
                        myMark = ESModElement.DislikeMark

                    if (index % 3 === 2)
                        myMark = ESModElement.LikeMarkNotFound

                    mainRectangle.ListView.view.likeBoxSignal(index, myMark, 23 + (index % 2), 36 - (index % 2))
                }
            }
        }

        Item {
            id: flagBox
            Layout.preferredHeight: layout3.implicitHeight
            Layout.preferredWidth: layout3.implicitWidth

            ColumnLayout {
                id: layout3
                Image {
                    source: "/icons/rus_flag.png"
                    sourceSize.width: Screen.pixelDensity * 3
                    sourceSize.height: Screen.pixelDensity * 2
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    visible: (langs.indexOf("Ru") > -1)
                }
                Image {
                    source: "/icons/eng_flag.png"
                    sourceSize.width: Screen.pixelDensity * 3
                    sourceSize.height: Screen.pixelDensity * 2
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    visible: (langs.indexOf("En") > -1)
                }
                Image {
                    source: "/icons/spa_flag.png"
                    sourceSize.width: Screen.pixelDensity * 3
                    sourceSize.height: Screen.pixelDensity * 2
                    Layout.preferredWidth: sourceSize.width
                    Layout.preferredHeight: sourceSize.height
                    visible: (langs.indexOf("Spa") > -1)
                }
            }
        }

        Image {
            id: delegateImage2
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            source: guiblocked == 2 ? "/icons/trash_press.png" : "/icons/trash.png"
            sourceSize.width: Screen.pixelDensity * 7
            sourceSize.height: Screen.pixelDensity * 7
            Layout.preferredWidth: sourceSize.width
            Layout.preferredHeight: sourceSize.height
            visible: (modstate === ESModElement.InstalledHasUpdate)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (guiblocked == ESModElement.NoBlock)
                        esModel.Delete(index)
                }
            }
        }
    }
}
