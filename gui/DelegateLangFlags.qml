import QtQuick 2.15
import QtQuick.Layouts 1.15


ColumnLayout {
    MMImage {
        source: "/icons/rus_flag.png"
        mmwidth: 3
        mmheight: 2
        visible: (model.langs.indexOf("Ru") > -1)
    }

    MMImage {
        source: "/icons/eng_flag.png"
        mmwidth: 3
        mmheight: 2
        visible: (model.langs.indexOf("En") > -1)
    }

    MMImage {
        source: "/icons/spa_flag.png"
        mmwidth: 3
        mmheight: 2
        visible: (model.langs.indexOf("Spa") > -1)
    }
}

