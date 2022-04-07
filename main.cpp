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


void RequestExternalStorageManager(void)
{
#if defined(ANDROID) && defined(EXTERNAL_STORAGE_MANAGER)
    jboolean is_esm = QAndroidJniObject::callStaticMethod<jboolean>("android/os/Environment", "isExternalStorageManager");

    QAndroidJniEnvironment env;
    if (env->ExceptionCheck())
    {
        QMessageBox::critical(nullptr, QString("Fail"), "Oops, isExternalStorageManager Exception found :(");
        env->ExceptionClear();
        is_esm = JNI_FALSE;
    }

    if (is_esm != JNI_FALSE)
        return;

    QAndroidIntent activityIntent("android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION");

    // unnecessary
    QAndroidJniObject param1 = QAndroidJniObject::fromString("android.intent.category.DEFAULT");
    activityIntent.handle().callObjectMethod("addCategory","(Ljava/lang/String;)Landroid/content/Intent;", param1.object<jstring>());

    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject packageName = activity.callObjectMethod("getPackageName", "()Ljava/lang/String;");
    // QMessageBox::information(nullptr, QString("Package name"), packageName.toString() + QString(" SDK:%1").arg(QtAndroid::androidSdkVersion()));

    QAndroidJniObject param2 = QAndroidJniObject::fromString("package:" + packageName.toString());
    QAndroidJniObject uri = QAndroidJniObject::callStaticObjectMethod("android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;", param2.object<jstring>());

    activityIntent.handle().callObjectMethod("setData", "(Landroid/net/Uri;)Landroid/content/Intent;", uri.object());

    QtAndroid::startActivity(activityIntent, 123);
#endif
}

void RequestExternalStorageReadWrite(void)
{
#ifdef ANDROID
    if(QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied ||
       QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
    {
        QtAndroid::requestPermissionsSync(QStringList({
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE"
        }));
    }
#endif
}

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

    RequestExternalStorageReadWrite();

    return app.exec();
}
