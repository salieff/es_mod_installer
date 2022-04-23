import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.salieff.esmodinstaller 1.0

Rectangle {
    width: locationButtonsLayout.implicitWidth + 40
    height: locationButtonsLayout.implicitHeight + 40
    anchors.centerIn: parent
    radius: 10
    color: "#f0ffffff"
    opacity: 0
    visible: false

    Behavior on opacity { NumberAnimation { duration: 300 } }

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
        mainWindow.hideAllPanels()

        mainLists.enabled = false
        visible = true
        opacity = 1
        mainAppTitle.closeButton.state = "CLOSE"
        return true
    }

    ButtonGroup { id: locationButtonsGroup }

    ColumnLayout {
        id: locationButtonsLayout
        anchors.centerIn: parent
        spacing: 20

        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Install mods to:")
        }

        RadioButton {
            id: buttonData
            checked: true
            text: qsTr("Data (Everlasting Summer Release)")
            ButtonGroup.group: locationButtonsGroup
            indicator: MMImage {
                source: "/icons/nolikes.png"
                mmwidth: 3
                mmheight: 3
                visible: buttonData.checked
                transformOrigin: Item.Center
                rotation: 180
                mirror: true
            }

            onToggled: {
                if (checked) {
                    busyIndicator.visible = true
                    busyIndicator.running = true
                    esModel.setModsInstallLocation(ESModModel.ModsInstallLocationData)
                }
            }
        }

        RadioButton {
            id: buttonMedia
            text: qsTr("Media (Everlasting Summer Beta)")
            ButtonGroup.group: locationButtonsGroup
            indicator: MMImage {
                source: "/icons/nolikes.png"
                mmwidth: 3
                mmheight: 3
                visible: buttonMedia.checked
                transformOrigin: Item.Center
                rotation: 180
                mirror: true
            }

            onToggled: {
                if (checked) {
                    busyIndicator.visible = true
                    busyIndicator.running = true
                    esModel.setModsInstallLocation(ESModModel.ModsInstallLocationMedia)
                }
            }
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        width: 100
        height: 100
        running: false;
        visible: false;
    }


    Connections {
        target: esModel
        function onCurrentModsInstallLocation(location) {
            if (location === ESModModel.ModsInstallLocationData) {
                buttonData.checked = true
                locationGear.source = "/icons/gear.png"
            } else {
                buttonMedia.checked = true
                locationGear.source = "/icons/gear_beta.png"
            }

            busyIndicator.running = false;
            busyIndicator.visible = false;
        }
    }
}
