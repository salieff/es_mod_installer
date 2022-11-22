import QtQuick 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: mainDelegateContainer

    property int margin: mm(1)

    height: mainLayout.implicitHeight + margin * 2
    width: ListView.view.width
    radius: 15
    border.width: 2
    border.color: "#22000000"
    color: "darkslategrey"

    DelegateProgress {}

    RowLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: parent.width - margin * 2
        spacing: parent.margin

        DelegateLeftButton {}
        DelegateHeart {}
        DelegateTitle {}
        DelegateLike {}
        DelegateLangFlags {}
        DelegateRightButton {}
    }
}
