import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls
import org.salieff.esmodinstaller


Rectangle {
    Layout.fillWidth: true
    Layout.bottomMargin: -radius
    Layout.preferredHeight: sortSearchLayout.implicitHeight + mm(2) + radius
    radius: mm(1)
    gradient: Gradient {
        GradientStop { position: 0; color: "#F0F0F0" }
        GradientStop { position: 1; color: "#909090" }
    }

    RowLayout {
        id: sortSearchLayout
        width: parent.width - mm(2)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.radius + mm(1)
        spacing: mm(1)

        Rectangle {
            width: mm(7)
            height: mm(7)
            radius: mm(1)
            gradient: Gradient {
                GradientStop { position: 0; color: "#FFFFFF" }
                GradientStop { position: 1; color: "#A0A0A0" }
            }

            Text {
                id: sortText
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 15
                style: Text.Raised
                styleColor: "white"
                text: "•"

                Connections {
                    target: esModel
                    function onListSorted(m) {
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

            MainSortMenu { id: mainSortMenu }

            MouseArea {
                anchors.fill: parent
                onClicked: mainSortMenu.popup()
            }
        }

        TextField {
            id: searchText
            Layout.fillWidth: true
            Layout.preferredHeight: mm(6)
            font.pointSize: 18
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
