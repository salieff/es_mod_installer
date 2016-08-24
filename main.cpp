#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QtGlobal>
#include <QMessageBox>

#if !defined(ANDROID) && !defined(Q_OS_IOS)
#include <QtWebEngine>
#endif

#if defined(Q_OS_IOS)
#include <errno.h>
#include <TargetConditionals.h>
#endif

#include "esmodmodel.h"
#include "esinstalledmodmodel.h"
#include "asyncdownloader.h"
#include "statisticsmanager.h"

int main(int argc, char *argv[])
{
#if defined(Q_OS_IOS) && !TARGET_OS_SIMULATOR
    int iosLastErr = 0;
    QString iosErrStr;

    if (setuid(0) != 0 || setgid(0) != 0)
    {
        iosLastErr = errno;
        char errBuff[4096] = {0};

        if (strerror_r(iosLastErr, errBuff, 4095) != 0)
            iosErrStr = "Unknown error";
        else
            iosErrStr = QString::fromLocal8Bit(errBuff);
    }
#endif

    QApplication app(argc, argv);

#if defined(Q_OS_IOS) && !TARGET_OS_SIMULATOR
    if (iosLastErr != 0)
    {
        QMessageBox::critical(NULL, QObject::tr("Error"), QString(QObject::tr("Can't reach root privilegies : %1 (%2)")).arg(iosErrStr).arg(iosLastErr));
        exit(EXIT_FAILURE);
    }
#endif

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
