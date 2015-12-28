#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QNetworkAccessManager>
#include <QNetworkProxy>

#include "esmodmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<ESModModel>("org.salieff.esmodinstaller", 1, 0, "ESModModel");
    qmlRegisterType<ESModElement>("org.salieff.esmodinstaller", 1, 0, "ESModElement");

    ESModModel esmodel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
