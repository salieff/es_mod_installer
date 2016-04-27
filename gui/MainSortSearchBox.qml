import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import org.salieff.esmodinstaller 1.0

Rectangle {
    height: sortSearchLayout.implicitHeight + 20 + radius
    anchors {
        bottom: parent.bottom
        bottomMargin: -radius
        left: parent.left
        right: parent.right
    }
    radius: 10
    gradient: Gradient {
        GradientStop { position: 0; color: "#F0F0F0" }
        GradientStop { position: 1; color: "#909090" }
    }

    RowLayout {
        id: sortSearchLayout
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.radius + 10
        spacing: 10

        Rectangle {
            width: Screen.pixelDensity * 7
            height: Screen.pixelDensity * 7
            radius: 10
            gradient: Gradient {
                GradientStop { position: 0; color: "#FFFFFF" }
                GradientStop { position: 1; color: "#A0A0A0" }
            }

            Text {
                id: sortText
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 25
                style: Text.Raised
                styleColor: "white"
                text: "•"

                Connections {
                    target: esModel
                    onListSorted: {
                        switch(m)
                        {
                        case ESModModel.AsServer :
                            sortText.text = "•"
                            break;

                        case ESModModel.ByNameUp :
                            sortText.text = "aZ↑"
                            break;

                        case ESModModel.ByNameDown :
                            sortText.text = "Za↓"
                            break;

                        case ESModModel.BySizeUp :
                            sortText.text = "Sz↑"
                            break;

                        case ESModModel.BySizeDown :
                            sortText.text = "Sz↓"
                            break;

                        case ESModModel.ByDateUp :
                            sortText.text = "Dt↑"
                            break;

                        case ESModModel.ByDateDown :
                            sortText.text = "Dt↓"
                            break;

                        case ESModModel.ByScore :
                            sortText.text = "5+"
                            break;

                        case ESModModel.ByVotesCount :
                            sortText.text = "웃"
                            break;

                        case ESModModel.ByActiveInstalls :
                            sortText.text = "I+"
                            break;

                        case ESModModel.ByTotalInstalls :
                            sortText.text = "I•"
                            break;

                        case ESModModel.ByLifeTime :
                            sortText.text = "Tm"
                            break;
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: mainSortMenu.popup()
            }
        }

        TextField {
            id: searchText
            Layout.fillWidth: true
            font.pointSize: 25
            placeholderText: qsTr("Name filter")

            onAccepted: {
                esModel.filterByKeywords(searchText.text)
                mainButtonSelector.forceActiveFocus()
            }
        }

        MMImage {
            source: "/icons/abort.png"
            mmwidth: 7
            mmheight: 7

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    searchText.text = ""
                    esModel.filterByKeywords(searchText.text)
                    mainButtonSelector.forceActiveFocus()
                }
            }
        }

        MMImage {
            source: "/icons/search.png"
            mmwidth: 7
            mmheight: 7

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    esModel.filterByKeywords(searchText.text)
                    mainButtonSelector.forceActiveFocus()
                }
            }

        }
    }
}
