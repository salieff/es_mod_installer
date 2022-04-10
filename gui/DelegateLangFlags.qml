import QtQuick 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    property var modeldata

    MMImage {
        source: "/icons/rus_flag.png"
        mmwidth: 3
        mmheight: 2
        visible: (modeldata.langs.indexOf("Ru") > -1)
    }

    MMImage {
        source: "/icons/eng_flag.png"
        mmwidth: 3
        mmheight: 2
        visible: (modeldata.langs.indexOf("En") > -1)
    }

    MMImage {
        source: "/icons/spa_flag.png"
        mmwidth: 3
        mmheight: 2
        visible: (modeldata.langs.indexOf("Spa") > -1)
    }
}

