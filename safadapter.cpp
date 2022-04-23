#include <QtAndroid>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <android/log.h>

#include "safadapter.h"
#include "safaccessdialog.h"


QString SafAdapter::m_currentAdapterRootSafPath = "Android/data";
std::map<QString, SafAdapter> SafAdapter::m_adaptersMap;
QMutex SafAdapter::m_adapterMutex;


QString SafAdapter::getCurrentAdapterRoot(void)
{
    QMutexLocker ml(&m_adapterMutex);
    return m_currentAdapterRootSafPath;
}

SafAdapter & SafAdapter::setCurrentAdapter(const QString &root)
{
    QMutexLocker ml(&m_adapterMutex);
    m_currentAdapterRootSafPath = root;

    return getAdapterForRoot(root);
}

SafAdapter & SafAdapter::getCurrentAdapter(void)
{
    QMutexLocker ml(&m_adapterMutex);
    return getAdapterForRoot(m_currentAdapterRootSafPath);
}

SafAdapter & SafAdapter::getAdapter(const QString &root)
{
    QMutexLocker ml(&m_adapterMutex);
    return getAdapterForRoot(root);
}

SafAdapter & SafAdapter::getAdapterForRoot(const QString &root)
{
    auto it = m_adaptersMap.find(root);
    if (it != m_adaptersMap.end())
        return it->second;

    auto pair = m_adaptersMap.emplace(root, root);
    pair.first->second.RequestRootUriPermissions();

    return pair.first->second;
}

SafAdapter::SafAdapter(const QString &rootSafPath) : m_rootSafPath(rootSafPath)
{
    if (CanUseNativeAPI())
    {
        QDir().mkpath(ConvertToNativePath(""));
        return;
    }

    m_javaSafAdapter = QAndroidJniObject("org/salieff/SafAdapter",
                                         "(Ljava/lang/String;)V",
                                         QAndroidJniObject::fromString(m_rootSafPath).object<jstring>());
}

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
    return m_javaSafAdapter.callMethod<jboolean>("rootUriPermissionGranted",
                                                 "(Landroid/content/Context;)Z",
                                                 QtAndroid::androidContext().object()) != JNI_FALSE;
}

void SafAdapter::RequestRootUriPermissions(void)
{
    if (CanUseNativeAPI())
        return;

    if (CheckRootUriPermissions())
        return;

    SafAccessDialog safAccessDialog;
    safAccessDialog.showMaximized();
    safAccessDialog.adjustImage();
    safAccessDialog.exec();

    auto intent = m_javaSafAdapter.callObjectMethod("intentForRootUriPermissionRequest",
                                                    "()Landroid/content/Intent;");

    QtAndroid::startActivity(intent, RootUriPermissionsRequestCode, [this](int receiverRequestCode, int resultCode, const QAndroidJniObject &intent) {
        if (receiverRequestCode != RootUriPermissionsRequestCode || resultCode != -1)
            return;

        m_javaSafAdapter.callMethod<void>("takeRootUriPermission",
                                          "(Landroid/content/Context;Landroid/content/Intent;)V",
                                          QtAndroid::androidContext().object(),
                                          intent.object());
    });
}

bool SafAdapter::CreateFolder(const QString &parentFolder, const QString &subFolder)
{
    if (CanUseNativeAPI())
        return QDir(ConvertToNativePath(parentFolder)).mkdir(subFolder);

    return m_javaSafAdapter.callMethod<jboolean>(
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
    if (CanUseNativeAPI())
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


        return open((ConvertToNativePath(parentFolder) + "/" + fileName).toLocal8Bit().constData(), flags);
    }

    return m_javaSafAdapter.callMethod<jint>(
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
    if (CanUseNativeAPI())
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


        return open((ConvertToNativePath(parentFolder) + "/" + fileName).toLocal8Bit().constData(), flags);
    }

    return m_javaSafAdapter.callMethod<jint>(
                "createFile",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(parentFolder).object<jstring>(),
                QAndroidJniObject::fromString(fileName).object<jstring>(),
                QAndroidJniObject::fromString(mode).object<jstring>()
                );
}

// Private
bool SafAdapter::CreateOrOpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, int (SafAdapter::*java_func)(const QString &parentFolder, const QString &fileName, const QString &mode), bool createFolders)
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

    int fd = (this->*java_func)(folderName, fileName, javaMode);
    return fd >= 0 && qf.open(fd, mode, QFileDevice::AutoCloseHandle);
}

bool SafAdapter::OpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders)
{
    return CreateOrOpenQFile(qf, filePath, mode, &SafAdapter::OpenFile, createFolders);
}

bool SafAdapter::CreateQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders)
{
    return CreateOrOpenQFile(qf, filePath, mode, &SafAdapter::CreateFile, createFolders);
}

bool SafAdapter::FileExists(const QString &filePath)
{
    if (CanUseNativeAPI())
        return QFile::exists(ConvertToNativePath(filePath));

    return FileSize(filePath) >= 0;
}

bool SafAdapter::FolderExists(const QString &folderPath)
{
    if (CanUseNativeAPI())
        return QDir(ConvertToNativePath(folderPath)).exists();

    return FolderSize(folderPath) >= 0;
}

// Private
int SafAdapter::FolderSize(const QString &folderPath)
{
    return m_javaSafAdapter.callMethod<jint>(
                "folderSize",
                "(Landroid/content/Context;Ljava/lang/String;)I",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(folderPath).object<jstring>()
                );
}

bool SafAdapter::DeleteFile(const QString &fileName)
{
    if (CanUseNativeAPI())
        return QFile(ConvertToNativePath(fileName)).remove();

    return m_javaSafAdapter.callMethod<jboolean>(
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

    while (!foldersList.empty() && !stopRootList.join("/").startsWith(foldersList.join("/")))
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
    if (CanUseNativeAPI())
        return QDir().rmdir(ConvertToNativePath(folderPath));

    return DeleteFile(folderPath);
}

bool SafAdapter::FolderEmpty(const QString &folderPath)
{
    if (CanUseNativeAPI())
        return QDir(ConvertToNativePath(folderPath)).isEmpty();

    return FolderSize(folderPath) == 0;
}

int64_t SafAdapter::FileSize(const QString &fileName)
{
    if (CanUseNativeAPI())
        return QFileInfo(ConvertToNativePath(fileName)).size();

    return m_javaSafAdapter.callMethod<jlong>(
                "fileSize",
                "(Landroid/content/Context;Ljava/lang/String;)J",
                QtAndroid::androidContext().object(),
                QAndroidJniObject::fromString(fileName).object<jstring>()
                );
}

bool SafAdapter::CanUseNativeAPI(void)
{
    return QtAndroid::androidSdkVersion() < MinimalSdkVersionForSaf;
}

QString SafAdapter::ConvertToNativePath(const QString &path)
{
    return "/sdcard/" + m_rootSafPath + path;
}
