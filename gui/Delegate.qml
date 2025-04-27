import QtQuick
import QtQuick.Layouts


Rectangle {
    id: mainDelegateContainer

    property int margin: mm(1)

    height: mainLayout.implicitHeight + margin * 2
    width: ListView.view.width
    radius: mm(1.5)
    border.width: mm(0.2)
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
