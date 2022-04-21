#ifndef SAFADAPTER_H
#define SAFADAPTER_H

#include <QString>
#include <QFile>
#include <QAndroidJniObject>
#include <QRandomGenerator>
#include <QMutex>

#include <map>

#include "minizip/ioapi.h"


class SafAdapter
{
public:
    static SafAdapter & setCurrentAdapter(const QString &root);
    static SafAdapter & getCurrentAdapter(void);
    static SafAdapter & getAdapter(const QString &root);
    static QString getCurrentAdapterRoot(void);

    static void RequestExternalStorageReadWrite(void);

    void RequestRootUriPermissions(void);

    bool CreateFolder(const QString &parentFolder, const QString &subFolder);
    bool CreateFoldersRecursively(const QString &foldersPath);
    bool DeleteEmptyFoldersRecursively(const QString &foldersPath, const QString &stopRootPath);
    bool FolderExists(const QString &folderPath);
    bool FolderEmpty(const QString &folderPath);
    bool DeleteFolder(const QString &folderPath);

    int OpenFile(const QString &parentFolder, const QString &fileName, const QString &mode);
    int CreateFile(const QString &parentFolder, const QString &fileName, const QString &mode);

    bool OpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders = false);
    bool CreateQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders = false);

    bool FileExists(const QString &filePath);
    bool DeleteFile(const QString &fileName);
    int64_t FileSize(const QString &fileName);

    static const bool CREATE_FOLDERS = true;
    static zlib_filefunc_def MiniZipFileAPI;

    SafAdapter(const QString &rootSafPath);

private:
    QString m_rootSafPath;
    QAndroidJniObject m_javaSafAdapter;
    int RootUriPermissionsRequestCode = QRandomGenerator::global()->generate();

    static const int MinimalSdkVersionForSaf = 29; // Android 10
    static QString m_currentAdapterRootSafPath;
    static QMutex m_adapterMutex;
    static std::map<QString, SafAdapter> m_adaptersMap;

    bool CheckRootUriPermissions(void);
    int FolderSize(const QString &folderPath);
    bool CreateOrOpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, int (SafAdapter::*java_func)(const QString &parentFolder, const QString &fileName, const QString &mode), bool createFolders);

    bool CanUseNativeAPI(void);
    QString ConvertToNativePath(const QString &path);

    static SafAdapter & getAdapterForRoot(const QString &root);
};

#endif // SAFADAPTER_H
