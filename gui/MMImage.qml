import QtQuick 2.15
import QtQuick.Layouts 1.15

Image {
    property int mmwidth
    property int mmheight

    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

    sourceSize.width: mm(mmwidth)
    sourceSize.height: mm(mmheight)

    Layout.preferredWidth: sourceSize.width
    Layout.preferredHeight: sourceSize.height
}
