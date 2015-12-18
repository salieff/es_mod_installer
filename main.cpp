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

    QNetworkAccessManager mgr;
#ifndef ANDROID
    mgr.setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 3128));
#endif
    ESModModel esmodel(&mgr);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    esmodel.setBusyIndicator(engine.rootObjects().first()->findChild<QObject *>("viewBusyIndicator"));
    esmodel.setAppTitleText(engine.rootObjects().first()->findChild<QObject *>("appTitleText"));

#ifdef ANDROID
    //QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    //window->showMaximized();
#else
    //window->resize(720, 900);
    //window->show();
#endif

    return app.exec();
}
