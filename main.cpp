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


ESModModel::ModsInstallLocation ConfiguredLocation(void)
{
    QFile f(AsyncJsonWriter::configFileName());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return ESModModel::ModsInstallLocationData;

    QByteArray data = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (doc.isNull())
        return ESModModel::ModsInstallLocationData;

    if (!doc.isObject())
        return ESModModel::ModsInstallLocationData;

    QJsonObject obj = doc.object();

    return static_cast<ESModModel::ModsInstallLocation>(obj["mods_install_location"].toInt(ESModModel::ModsInstallLocationData));
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SafAdapter::RequestExternalStorageReadWrite();

    if (ConfiguredLocation() == ESModModel::ModsInstallLocationData)
        SafAdapter::setCurrentAdapter("Android/data");
    else
        SafAdapter::setCurrentAdapter("Android/media");

    AsyncDownloader::createNetworkManager(&app);
    StatisticsManager::getInstance(&app);

    QtWebView::initialize();

    qmlRegisterType<ESModModel>("org.salieff.esmodinstaller", 1, 0, "ESModModel");
    qmlRegisterType<ESModElement>("org.salieff.esmodinstaller", 1, 0, "ESModElement");

    ESModModel esmodel;

    ESInstalledModModel esInstalledModel;
    esInstalledModel.setSourceModel(&esmodel);

    ESInstalledModModel esServerModel(true);
    esServerModel.setSourceModel(&esmodel);

    ESIncompletedModModel esIncompletedModel;
    esIncompletedModel.setSourceModel(&esmodel);

    ESBrokenModModel esBrokenModel;
    esBrokenModel.setSourceModel(&esmodel);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.rootContext()->setContextProperty("esInstalledModel", &esInstalledModel);
    engine.rootContext()->setContextProperty("esServerModel", &esServerModel);
    engine.rootContext()->setContextProperty("esIncompletedModel", &esIncompletedModel);
    engine.rootContext()->setContextProperty("esBrokenModel", &esBrokenModel);
    engine.load(QUrl(QStringLiteral("qrc:/gui/main.qml")));

    return app.exec();
}
