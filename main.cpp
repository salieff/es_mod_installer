#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QtGlobal>
#include <QMessageBox>

#include <QtWebView>
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidIntent>
#include <QAndroidJniObject>

#include "esmodmodel.h"
#include "esinstalledmodmodel.h"
#include "asyncdownloader.h"
#include "statisticsmanager.h"
#include "safadapter.h"
#include "downloadservice.h"


int main(int argc, char *argv[])
{
    QtWebView::initialize();

    QApplication app(argc, argv);

    SafAdapter::RequestExternalStorageReadWrite();
    SafAdapter::getCurrentAdapter();

    DownloadService::RegisterJNINativeMethods();
    DownloadService::SyncDownloads();

    AsyncDownloader::createNetworkManager(&app);
    StatisticsManager::getInstance(&app);

    qmlRegisterType<ESModModel>("org.salieff.esmodinstaller", 1, 0, "ESModModel");
    qmlRegisterType<ESModElement>("org.salieff.esmodinstaller", 1, 0, "ESModElement");

    ESModModel esmodel;

    ESInstalledModModel esInstalledModel(&esmodel);
    // ESInstalledModModel esServerModel(&esmodel, true);
    // ESReleasedModModel esReleasedModel(&esmodel);
    ESIncompletedModModel esIncompletedModel(&esmodel);
    ESBrokenModModel esBrokenModel(&esmodel);
    ESFavoriteModModel esFavoriteModel(&esmodel);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.rootContext()->setContextProperty("esInstalledModel", &esInstalledModel);
    // engine.rootContext()->setContextProperty("esServerModel", &esServerModel);
    // engine.rootContext()->setContextProperty("esReleasedModel", &esReleasedModel);
    engine.rootContext()->setContextProperty("esIncompletedModel", &esIncompletedModel);
    engine.rootContext()->setContextProperty("esBrokenModel", &esBrokenModel);
    engine.rootContext()->setContextProperty("esFavoriteModel", &esFavoriteModel);
    engine.load(QUrl(QStringLiteral("qrc:/gui/main.qml")));

    return app.exec();
}
