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

#if 0
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

        if (SafAdapter::FileExists("Android/data/su.sovietgames.everlasting_summer/files/test_test_test/test_file.txt"))
        {
            QMessageBox::information(nullptr, QString("Pass"), "SafAdapter::CreateFile already exists");
        }
        else
        {
            int fd = SafAdapter::CreateFile("Android/data/su.sovietgames.everlasting_summer/files/test_test_test", "test_file.txt");
            if (fd < 0)
                QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::CreateFile :(");

            if (::write(fd, "Hello!", 6) != 6)
                QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't ::write(fd) :(");

            ::close(fd);

            QMessageBox::information(
                        nullptr,
                        QString("File size:"),
                        QString("%1")
                        .arg(SafAdapter::FileSize("Android/data/su.sovietgames.everlasting_summer/files/test_test_test/test_file.txt"))
                        );
        }

        if (!SafAdapter::CreateFoldersRecursively("Android/data/su.sovietgames.everlasting_summer/files/test1/test2/test3/test4/test5"))
            QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::CreateFoldersRecursively :(");

        QMessageBox::information(nullptr, QString("Pass"), "Now you can check data folder");

        if (!SafAdapter::DeleteEmptyFoldersRecursively("Android/data/su.sovietgames.everlasting_summer/files/test1/test2/test3/test4/test5",
                                                  "Android/data/su.sovietgames.everlasting_summer/files/"))
            QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::DeleteFoldersRecursively :(");

        if (!SafAdapter::DeleteFile("Android/data/su.sovietgames.everlasting_summer/files/test_test_test/test_file.txt"))
            QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::DeleteFile :(");

        if (!SafAdapter::FolderEmpty("Android/data/su.sovietgames.everlasting_summer/files/test_test_test"))
            QMessageBox::warning(nullptr, QString("Warning"), "We are trying to delete non-empty Folder!");

        if (!SafAdapter::DeleteFile("Android/data/su.sovietgames.everlasting_summer/files/test_test_test"))
            QMessageBox::critical(nullptr, QString("Fail"), "Oops, can't SafAdapter::DeleteFile(Folder) :(");

        QMessageBox::information(nullptr, QString("Pass"), "All test passed");
    }
#endif

    return app.exec();
}
