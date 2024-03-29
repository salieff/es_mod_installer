import QtQuick 2.15
import QtQuick.Controls 1.4
import org.salieff.esmodinstaller 1.0

Menu {
    title: qsTr("Sort mode")

    MenuItem {
        text: qsTr("As server")
        onTriggered: esModel.sortList(ESModModel.AsServer)
    }

    MenuItem {
        text: qsTr("By status")
        onTriggered: esModel.sortList(ESModModel.ByStatus)
    }

    MenuItem {
        text: qsTr("By name a→Z")
        onTriggered: esModel.sortList(ESModModel.ByNameUp)
    }

    MenuItem {
        text: qsTr("By name Z→a")
        onTriggered: esModel.sortList(ESModModel.ByNameDown)
    }

    MenuItem {
        text: qsTr("By size 1→99")
        onTriggered: esModel.sortList(ESModModel.BySizeUp)
    }

    MenuItem {
        text: qsTr("By size 99→1")
        onTriggered: esModel.sortList(ESModModel.BySizeDown)
    }

    MenuItem {
        text: qsTr("By date 1.1.1970 → 12.12.2015")
        onTriggered: esModel.sortList(ESModModel.ByDateUp)
    }

    MenuItem {
        text: qsTr("By date 12.12.2015 → 1.1.1970")
        onTriggered: esModel.sortList(ESModModel.ByDateDown)
    }

    MenuItem {
        text: qsTr("By score")
        onTriggered: esModel.sortList(ESModModel.ByScore)
    }

    MenuItem {
        text: qsTr("By votes count")
        onTriggered: esModel.sortList(ESModModel.ByVotesCount)
    }

    MenuItem {
        text: qsTr("By active installs")
        onTriggered: esModel.sortList(ESModModel.ByActiveInstalls)
    }

    MenuItem {
        text: qsTr("By total installs")
        onTriggered: esModel.sortList(ESModModel.ByTotalInstalls)
    }

    MenuItem {
        text: qsTr("By hours of life")
        onTriggered: esModel.sortList(ESModModel.ByLifeTime)
    }
}
