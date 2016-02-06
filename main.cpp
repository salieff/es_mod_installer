#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QNetworkAccessManager>
#include <QNetworkProxy>

#ifndef ANDROID
#include <QtWebEngine>
#endif

#include "esmodmodel.h"
#include "asyncdownloader.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AsyncDownloader::NetworkManager = new QNetworkAccessManager(&app);

#ifndef ANDROID
    QtWebEngine::initialize();
#endif

    qmlRegisterType<ESModModel>("org.salieff.esmodinstaller", 1, 0, "ESModModel");
    qmlRegisterType<ESModElement>("org.salieff.esmodinstaller", 1, 0, "ESModElement");

    ESModModel esmodel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.load(QUrl(QStringLiteral("qrc:/gui/main.qml")));

    return app.exec();
}
