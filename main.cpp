#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QtGlobal>
#include <QMessageBox>

#if !defined(ANDROID) && !defined(Q_OS_IOS)
#include <QtWebEngine>
#endif

#include "esmodmodel.h"
#include "esinstalledmodmodel.h"
#include "asyncdownloader.h"
#include "statisticsmanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AsyncDownloader::createNetworkManager(&app);
    StatisticsManager::getInstance(&app);

#if !defined(ANDROID) && !defined(Q_OS_IOS)
    QtWebEngine::initialize();
#endif

    qmlRegisterType<ESModModel>("org.salieff.esmodinstaller", 1, 0, "ESModModel");
    qmlRegisterType<ESModElement>("org.salieff.esmodinstaller", 1, 0, "ESModElement");

    ESModModel esmodel;

    ESInstalledModModel esInstalledModel;
    esInstalledModel.setSourceModel(&esmodel);

    ESInstalledModModel esServerModel(true);
    esServerModel.setSourceModel(&esmodel);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.rootContext()->setContextProperty("esInstalledModel", &esInstalledModel);
    engine.rootContext()->setContextProperty("esServerModel", &esServerModel);
    engine.load(QUrl(QStringLiteral("qrc:/gui/main.qml")));

    return app.exec();
}
