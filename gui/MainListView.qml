import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.salieff.esmodinstaller 1.0

ListView {
    id: listView

    property string headerText
    property bool initialScrollDone: false

    delegate: Delegate {}

    spacing: 5
    maximumFlickVelocity: 7000

    ScrollBar.vertical: EsScrollBar {}

    header: Item {
        // Empty placeholder
        width: parent.width
        height: hdrRect.height + 10
        anchors.horizontalCenter: parent.horizontalCenter
    }

    remove: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0; duration: 400 }
        NumberAnimation { property: "scale"; from: 1.0; to: 0; duration: 400 }
    }

    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutBounce }
    }

    Item {
        z: 2
        width: parent.width
        height: hdrRect.height + 10
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.75

        Rectangle {
            id: hdrRect
            width: hdrText.contentWidth + 20
            height: hdrText.contentHeight + 20
            anchors.horizontalCenter: parent.horizontalCenter
            radius: 10
            gradient: Gradient {
                GradientStop { position: 0; color: "#FFFFFF" }
                GradientStop { position: 1; color: "#A0A0A0" }
            }

            Text {
                id: hdrText
                anchors.centerIn: parent
                font.pointSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                style: Text.Raised
                styleColor: "white"
                text: headerText
            }
        }
    }

    NumberAnimation {
        id: scrollAnim
        target: listView
        property: "contentY"
        duration: 500
        easing.type: Easing.InOutQuad
        property int targetIndex: -1
        property int targetMode: ListView.Beginning
        onFinished: listView.positionViewAtIndex(targetIndex, targetMode)   // точная доводка в конце
    }

    function animatedScrollToIndex(index, mode) {
        scrollAnim.stop()
        cancelFlick()

        var from = contentY
        positionViewAtIndex(index, mode)      // мгновенно ставим в конечную позицию...
        var to = contentY
        contentY = from                       // ...и тут же возвращаем обратно

        scrollAnim.targetIndex = index
        scrollAnim.targetMode = mode
        scrollAnim.from = from
        scrollAnim.to = to
        scrollAnim.restart()
    }

    onMovementStarted: scrollAnim.stop()      // пользователь схватил список: не мешаем ему

    function scrollToFirstMatch() {
        if (initialScrollDone || count === 0)
            return

        var row = model.firstRowWhere("modstate", ESModElement.Downloading)

        if (row < 0)
            row = model.firstRowWhere("modstate", ESModElement.Unpacking)

        if (row < 0)
            return

        listView.animatedScrollToIndex(row, ListView.Center)
        initialScrollDone = true
    }

    Connections {
        target: esModel
        function onEsIndexReceived() {
            Qt.callLater(listView.scrollToFirstMatch)
        }
    }
}
