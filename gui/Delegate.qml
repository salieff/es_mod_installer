import QtQuick 2.5
import QtQuick.Layouts 1.2

Rectangle {
    id: mainDelegateContainer

    property int margin: 10

    anchors.horizontalCenter: parent.horizontalCenter
    height: mainLayout.implicitHeight + margin * 2
    width: parent.width
    radius: 15
    border.width: 2
    border.color: "#22000000"
    color: "darkslategrey"

    DelegateProgress {
        modeldata: model
    }

    RowLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: parent.width - margin * 2
        spacing: parent.margin

        DelegateLeftButton {
            modeldata: model
        }

        DelegateTitle {
            modeldata: model
        }

        DelegateLike {
            modeldata: model
        }

        DelegateLangFlags {
            modeldata: model
        }

        DelegateRightButton {
            modeldata: model
        }

    }
}
