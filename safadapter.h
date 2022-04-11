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
    static bool CheckRootUriPermissions(void);
    static void RequestRootUriPermissions(void);
    static bool CreateFolder(const QString &parentFolder, const QString &subFolder);
    static bool CreateFoldersRecursively(const QString &foldersPath);
    static int OpenFile(const QString &parentFolder, const QString &fileName, const QString &mode);
    static int CreateFile(const QString &parentFolder, const QString &fileName, const QString &mode);
    static bool OpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders = false);
    static bool CreateQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, bool createFolders = false);
    static bool FileExists(const QString &fileName);
    static bool DeleteFile(const QString &fileName);
    static bool DeleteEmptyFoldersRecursively(const QString &foldersPath, const QString &stopRootPath);
    static bool FolderEmpty(const QString &folderName);
    static int64_t FileSize(const QString &fileName);

    const static bool CREATE_FOLDERS = true;
    static zlib_filefunc_def MiniZipFileAPI;

private:
    const static int RootUriPermissionsRequestCode = 513375;
    static bool CreateOrOpenQFile(QFile &qf, const QString &filePath, QIODevice::OpenMode mode, int (*java_func)(const QString &parentFolder, const QString &fileName, const QString &mode), bool createFolders);
};

#endif // SAFADAPTER_H
