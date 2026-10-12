#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QtGlobal>

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
#include "admcontroller.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QtWebView::initialize();

    SafAdapter::RequestExternalStorageReadWrite();
    SafAdapter::getCurrentAdapter();

    qRegisterMetaType<ADMController::Status>();
    qRegisterMetaType<ADMController::Reason>();
    ADMController::Initialize();

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

    if (!SafAdapter::getCurrentAdapter().FileExists(".nomedia"))
        close(SafAdapter::getCurrentAdapter().CreateFile("", ".nomedia", "wt"));

    auto push = [](Qt::ApplicationState state) {
        QAndroidJniObject::callStaticMethod<void>("org/salieff/AppState", "setActive", "(Z)V", jboolean(state == Qt::ApplicationActive));

        if (state != Qt::ApplicationActive)
            return;

        // Уберем все нотификации
        QAndroidJniObject nm = QtAndroid::androidContext().callObjectMethod(
            "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
            QAndroidJniObject::fromString("notification").object<jstring>());   // Context.NOTIFICATION_SERVICE

        if (nm.isValid())
            nm.callMethod<void>("cancelAll");

        QAndroidJniEnvironment env;
        if (env->ExceptionCheck()) // на случай ошибки JNI
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }

    };

    push(Qt::ApplicationActive);   // начальное состояние
    QObject::connect(&app, &QGuiApplication::applicationStateChanged, push);

    return app.exec();
}
