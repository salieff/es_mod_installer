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

#include "esmodmodel.h"
#include "esinstalledmodmodel.h"
#include "asyncdownloader.h"
#include "statisticsmanager.h"
#include "safadapter.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

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

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("esModel", &esmodel);
    engine.rootContext()->setContextProperty("esInstalledModel", &esInstalledModel);
    engine.rootContext()->setContextProperty("esServerModel", &esServerModel);
    engine.rootContext()->setContextProperty("esIncompletedModel", &esIncompletedModel);
    engine.load(QUrl(QStringLiteral("qrc:/gui/main.qml")));

    SafAdapter::RequestExternalStorageReadWrite();
    SafAdapter::RequestRootUriPermissions();

    if (SafAdapter::CheckRootUriPermissions())
    {
        if (SafAdapter::FileExists("Android/data/su.sovietgames.everlasting_summer/files/test_test_test"))
        {
            QMessageBox::information(nullptr, QString("Pass"), "SafAdapter::CreateFolder already exists");
        }
        else
        {
            if (!SafAdapter::CreateFolder("Android/data/su.sovietgames.everlasting_summer/files", "test_test_test"))
                QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::CreateFolder :(");
        }

        if (SafAdapter::FileExists("Android/data/su.sovietgames.everlasting_summer/files/test_test_test/test_file.zip"))
        {
            QMessageBox::information(nullptr, QString("Pass"), "SafAdapter::CreateFile already exists");
        }
        else
        {
            if (!SafAdapter::CreateFile("Android/data/su.sovietgames.everlasting_summer/files/test_test_test", "test_file.zip").isValid())
                QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::CreateFile :(");
        }

        if (!SafAdapter::DeleteFile("Android/data/su.sovietgames.everlasting_summer/files/test_test_test/test_file.zip"))
            QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::DeleteFile :(");

        if (!SafAdapter::DeleteFile("Android/data/su.sovietgames.everlasting_summer/files/test_test_test"))
            QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::DeleteFile(Folder) :(");

        QMessageBox::information(nullptr, QString("Pass"), "All test passed");
    }

    return app.exec();
}
