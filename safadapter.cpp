#include <QtAndroid>
#include <QFileInfo>
#include <QDir>

#include "safadapter.h"
#include "safaccessdialog.h"


void SafAdapter::RequestExternalStorageReadWrite(void)
{
    if(QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") != QtAndroid::PermissionResult::Granted ||
       QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE") != QtAndroid::PermissionResult::Granted)
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

    SafAccessDialog safAccessDialog;
    safAccessDialog.adjustSize();
    safAccessDialog.exec();

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

bool SafAdapter::CreateFoldersRecursively(const QString &foldersPath)
{
    QString createdFoldersPath;
    auto foldersList = foldersPath.split('/', Qt::SkipEmptyParts);
    for (const auto &folderName : foldersList)
    {
        if (!FileExists(createdFoldersPath + "/" + folderName))
            if (!CreateFolder(createdFoldersPath, folderName))
                return false;

        createdFoldersPath += "/" + folderName;
    }

    return true;
}

int SafAdapter::OpenFile(const QString &parentFolder, const QString &fileName, const QString &mode)
{
    return QAndroidJniObject::callStaticMethod<jint>(
                "org/salieff/SafAdapter",
                "openFile",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(parentFolder).object<jstring>(),
                QAndroidJniObject::fromString(fileName).object<jstring>(),
                QAndroidJniObject::fromString(mode).object<jstring>()
                );
}

int SafAdapter::CreateFile(const QString &parentFolder, const QString &fileName, const QString &mode)
{
    return QAndroidJniObject::callStaticMethod<jint>(
                "org/salieff/SafAdapter",
                "createFile",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(parentFolder).object<jstring>(),
                QAndroidJniObject::fromString(fileName).object<jstring>(),
                QAndroidJniObject::fromString(mode).object<jstring>()
                );
}

bool SafAdapter::CreateOrOpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, int (*java_func)(const QString &parentFolder, const QString &fileName, const QString &mode), bool createFolders)
{
    QString folderName = QFileInfo(filePath).dir().path();
    QString fileName = QFileInfo(filePath).fileName();

    if (createFolders && !CreateFoldersRecursively(folderName))
        return false;

    QString javaMode;

    if (mode & QIODevice::ReadOnly)
        javaMode += "r";

    if (mode & QIODevice::WriteOnly)
        javaMode += "w";

    if (mode & QIODevice::Truncate)
        javaMode += "t";

    if (mode & QIODevice::Append)
        javaMode += "a";

    int fd = java_func(folderName, fileName, javaMode);
    return fd >= 0 && qf.open(fd, mode, QFileDevice::AutoCloseHandle);
}

bool SafAdapter::OpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders)
{
    return CreateOrOpenQFile(qf, filePath, mode, OpenFile, createFolders);
}

bool SafAdapter::CreateQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders)
{
    return CreateOrOpenQFile(qf, filePath, mode, CreateFile, createFolders);
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

bool SafAdapter::DeleteEmptyFoldersRecursively(const QString &foldersPath, const QString &stopRootPath)
{
    auto foldersList = foldersPath.split('/', Qt::SkipEmptyParts);
    auto stopRootList = stopRootPath.split('/', Qt::SkipEmptyParts);

    while (!foldersList.empty() && foldersList != stopRootList)
    {
        auto currentPath = foldersList.join("/");

        if (!FolderEmpty(currentPath))
            return false;

        if (!DeleteFile(currentPath))
            return false;

        foldersList.pop_back();
    }

    return true;
}

bool SafAdapter::FolderEmpty(const QString &folderName)
{
    return QAndroidJniObject::callStaticMethod<jboolean>(
                "org/salieff/SafAdapter",
                "folderEmpty",
                "(Landroid/content/Context;Ljava/lang/String;)Z",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(folderName).object<jstring>()
                ) != JNI_FALSE;
}

int64_t SafAdapter::FileSize(const QString &fileName)
{
    return QAndroidJniObject::callStaticMethod<jlong>(
                "org/salieff/SafAdapter",
                "fileSize",
                "(Landroid/content/Context;Ljava/lang/String;)J",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(fileName).object<jstring>()
                );
}
