import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2

Image {
    property int mmwidth
    property int mmheight

    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

    sourceSize.width: Screen.pixelDensity * mmwidth
    sourceSize.height: Screen.pixelDensity * mmheight
    Layout.preferredWidth: sourceSize.width
    Layout.preferredHeight: sourceSize.height
}
