#ifndef SAFADAPTER_H
#define SAFADAPTER_H

#include <QString>
#include <QFile>
#include <QAndroidJniObject>

#include "minizip/ioapi.h"

class SafAdapter
{
public:
    static void RequestExternalStorageReadWrite(void);
    static void RequestRootUriPermissions(void);

    static bool CreateFolder(const QString &parentFolder, const QString &subFolder);
    static bool CreateFoldersRecursively(const QString &foldersPath);
    static bool DeleteEmptyFoldersRecursively(const QString &foldersPath, const QString &stopRootPath);
    static bool FolderExists(const QString &folderPath);
    static bool FolderEmpty(const QString &folderPath);
    static bool DeleteFolder(const QString &folderPath);

    static int OpenFile(const QString &parentFolder, const QString &fileName, const QString &mode);
    static int CreateFile(const QString &parentFolder, const QString &fileName, const QString &mode);

    static bool OpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders = false);
    static bool CreateQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders = false);

    static bool FileExists(const QString &filePath);
    static bool DeleteFile(const QString &fileName);
    static int64_t FileSize(const QString &fileName);

    const static bool CREATE_FOLDERS = true;
    static zlib_filefunc_def MiniZipFileAPI;

private:
    static const int RootUriPermissionsRequestCode = 513375;
    static const int MinimalSdkVersionForSaf = 29; // Android 10
    static const char *NativePathPrefix;

    static bool CheckRootUriPermissions(void);
    static int FolderSize(const QString &folderPath);
    static bool CreateOrOpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, int (*java_func)(const QString &parentFolder, const QString &fileName, const QString &mode), bool createFolders);
};

#endif // SAFADAPTER_H
