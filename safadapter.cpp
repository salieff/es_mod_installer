#include <QtAndroid>
#include <QFileInfo>
#include <QDir>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "safadapter.h"
#include "safaccessdialog.h"

static const char *NativePathPrefix = "/sdcard/Android";


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

// Private
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
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return;

    if (CheckRootUriPermissions())
        return;

    SafAccessDialog safAccessDialog;
    safAccessDialog.show();
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
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QDir(NativePathPrefix + parentFolder).mkdir(subFolder);

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
        if (!FolderExists(createdFoldersPath + "/" + folderName))
            if (!CreateFolder(createdFoldersPath, folderName))
                return false;

        createdFoldersPath += "/" + folderName;
    }

    return true;
}

int SafAdapter::OpenFile(const QString &parentFolder, const QString &fileName, const QString &mode)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
    {
        int flags = 0;

        if (mode.contains('r') && mode.contains('w'))
            flags = O_RDWR;
        else if (mode.contains('r'))
            flags = O_RDONLY;
        else if (mode.contains('w'))
            flags = O_WRONLY;

        if (mode.contains('a'))
            flags |= O_APPEND;

        if (mode.contains('t'))
            flags |= O_TRUNC;


        return open((NativePathPrefix + parentFolder + "/" + fileName).toLocal8Bit().constData(), flags);
    }

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
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
    {
        int flags = 0;

        if (mode.contains('r') && mode.contains('w'))
            flags = O_RDWR;
        else if (mode.contains('r'))
            flags = O_RDONLY;
        else if (mode.contains('w'))
            flags = O_WRONLY;

        flags |= O_CREAT;

        if (mode.contains('a'))
            flags |= O_APPEND;

        if (mode.contains('t'))
            flags |= O_TRUNC;


        return open((NativePathPrefix + parentFolder + "/" + fileName).toLocal8Bit().constData(), flags);
    }

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

// Private
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

bool SafAdapter::FileExists(const QString &filePath)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QFile::exists(NativePathPrefix + filePath);

    QString folderName = QFileInfo(filePath).dir().path();
    QString fileName = QFileInfo(filePath).fileName();

    int fd = OpenFile(folderName, fileName, "r");
    close(fd);

    return fd >= 0;
}

bool SafAdapter::FolderExists(const QString &folderPath)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QDir(NativePathPrefix + folderPath).exists();

    return FolderSize(folderPath) >= 0;
}

// Private
int SafAdapter::FolderSize(const QString &folderPath)
{
    return QAndroidJniObject::callStaticMethod<jint>(
                "org/salieff/SafAdapter",
                "folderSize",
                "(Landroid/content/Context;Ljava/lang/String;)I",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(folderPath).object<jstring>()
                );
}

bool SafAdapter::DeleteFile(const QString &fileName)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QFile(NativePathPrefix + fileName).remove();

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
        auto currentPath = "/" + foldersList.join("/");

        if (!FolderEmpty(currentPath))
            return false;

        if (!DeleteFolder(currentPath))
            return false;

        foldersList.pop_back();
    }

    return true;
}

bool SafAdapter::DeleteFolder(const QString &folderPath)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QDir().rmdir(NativePathPrefix + folderPath);

    return DeleteFile(folderPath);
}

bool SafAdapter::FolderEmpty(const QString &folderPath)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QDir(NativePathPrefix + folderPath).isEmpty();

    return FolderSize(folderPath) == 0;
}

int64_t SafAdapter::FileSize(const QString &fileName)
{
    if (QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf)
        return QFileInfo(NativePathPrefix + fileName).size();

    return QAndroidJniObject::callStaticMethod<jlong>(
                "org/salieff/SafAdapter",
                "fileSize",
                "(Landroid/content/Context;Ljava/lang/String;)J",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(fileName).object<jstring>()
                );
}
