#ifndef SAFADAPTER_H
#define SAFADAPTER_H

#include <QString>
#include <QAndroidJniObject>


class SafAdapter
{
public:
    static void RequestExternalStorageReadWrite(void);
    static bool CheckRootUriPermissions(void);
    static void RequestRootUriPermissions(void);
    static bool CreateFolder(const QString &parentFolder, const QString &subFolder);
    static QAndroidJniObject CreateFile(const QString &parentFolder, const QString &fileName);
    static bool FileExists(const QString &fileName);
    static bool DeleteFile(const QString &fileName);

private:
    const static int RootUriPermissionsRequestCode = 513375;
};

#endif // SAFADAPTER_H
