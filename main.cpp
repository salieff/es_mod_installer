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

    ESModModel esmodel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    esmodel.setBusyIndicator(engine.rootObjects().first()->findChild<QObject *>("viewBusyIndicator"));
    esmodel.setAppTitleText(engine.rootObjects().first()->findChild<QObject *>("appTitleText"));

    return app.exec();
}
