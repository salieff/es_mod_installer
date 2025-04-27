import QtQuick
import QtQuick.Layouts


Image {
    property int mmwidth
    property int mmheight

    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

    sourceSize.width: mm(mmwidth)
    sourceSize.height: mm(mmheight)

    Layout.preferredWidth: sourceSize.width
    Layout.preferredHeight: sourceSize.height
}
