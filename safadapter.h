#ifndef SAFADAPTER_H
#define SAFADAPTER_H

#include <QString>
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
    static int CreateFile(const QString &parentFolder, const QString &fileName);
    static bool FileExists(const QString &fileName);
    static bool DeleteFile(const QString &fileName);
    static bool DeleteEmptyFoldersRecursively(const QString &foldersPath, const QString &stopRootPath);
    static bool FolderEmpty(const QString &folderName);
    static int64_t FileSize(const QString &fileName);

    static zlib_filefunc_def MiniZipFileAPI;

private:
    const static int RootUriPermissionsRequestCode = 513375;
};

#endif // SAFADAPTER_H
