#include <QtAndroid>

#include "safadapter.h"


void SafAdapter::RequestExternalStorageReadWrite(void)
{
    if(QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied ||
       QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
    {
        QtAndroid::requestPermissionsSync(QStringList({
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE"
        }));
    }
}

bool SafAdapter::CheckRootUriPermissions(void)
{
    return QAndroidJniObject::callStaticMethod<jboolean>(
                "org/salieff/SafAdapter",
                "rootUriPermissionGranted",
                "(Landroid/content/Context;)Z",
                QtAndroid::androidContext().object()
                ) != JNI_FALSE;
}

void SafAdapter::RequestRootUriPermissions(void)
{
    if (CheckRootUriPermissions())
        return;

    auto intent = QAndroidJniObject::callStaticObjectMethod("org/salieff/SafAdapter",
                                                            "intentForRootUriPermissionRequest",
                                                            "()Landroid/content/Intent;");

    QtAndroid::startActivity(intent, RootUriPermissionsRequestCode, [](int receiverRequestCode, int resultCode, const QAndroidJniObject &intent) {
        if (receiverRequestCode != RootUriPermissionsRequestCode || resultCode != -1)
            return;

        QAndroidJniObject::callStaticMethod<void>("org/salieff/SafAdapter",
                                                  "takeRootUriPermission",
                                                  "(Landroid/content/Context;Landroid/content/Intent;)V",
                                                  QtAndroid::androidContext().object(),
                                                  intent.object());
    });
}

bool SafAdapter::CreateFolder(const QString &parentFolder, const QString &subFolder)
{
    return QAndroidJniObject::callStaticMethod<jboolean>(
                "org/salieff/SafAdapter",
                "createFolder",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)Z",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(parentFolder).object<jstring>(),
                QAndroidJniObject::fromString(subFolder).object<jstring>()
                ) != JNI_FALSE;
}

QAndroidJniObject SafAdapter::CreateFile(const QString &parentFolder, const QString &fileName)
{
    return QAndroidJniObject::callStaticObjectMethod(
                "org/salieff/SafAdapter",
                "createFile",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(parentFolder).object<jstring>(),
                QAndroidJniObject::fromString(fileName).object<jstring>()
                );
}

bool SafAdapter::FileExists(const QString &fileName)
{
    return QAndroidJniObject::callStaticMethod<jboolean>(
                "org/salieff/SafAdapter",
                "fileExists",
                "(Landroid/content/Context;Ljava/lang/String;)Z",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(fileName).object<jstring>()
                ) != JNI_FALSE;
}

bool SafAdapter::DeleteFile(const QString &fileName)
{
    return QAndroidJniObject::callStaticMethod<jboolean>(
                "org/salieff/SafAdapter",
                "deleteFile",
                "(Landroid/content/Context;Ljava/lang/String;)Z",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(fileName).object<jstring>()
                ) != JNI_FALSE;
}
