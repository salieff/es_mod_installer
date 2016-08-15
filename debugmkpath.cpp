#include "debugmkpath.h"

#include <QDir>
#include <QFile>

#include <qplatformdefs.h>

#include <errno.h>
#include <string.h>

static void fillErrorString(int e, QString path, QString *errStr)
{
    if (errStr == NULL)
        return;

    char buff[4096] = {0};
    QString s;

    if (strerror_r(e, buff, 4095) != 0)
        s = "Unknown error";
    else
        s = QString::fromLocal8Bit(buff);

    *errStr = QString("Can't create directory %1 : %2 (%3)").arg(path).arg(s).arg(e);
}

bool DebugMkPath(QString dirName, QString *errStr)
{
    dirName = QDir::cleanPath(dirName);
    for (int oldslash = -1, slash = 0; slash != -1; oldslash = slash)
    {
        slash = dirName.indexOf(QDir::separator(), oldslash + 1);
        if (slash == -1)
        {
            if (oldslash == dirName.length())
                break;

            slash = dirName.length();
        }

        if (slash)
        {
            const QByteArray chunk = QFile::encodeName(dirName.left(slash));
            if (QT_MKDIR(chunk.constData(), 0777) != 0)
            {
                int storedErrno = errno;

                if (errno == EEXIST)
                {
                    QT_STATBUF st;
                    if (QT_STAT(chunk.constData(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR)
                        continue;
                }

                fillErrorString(storedErrno, dirName.left(slash), errStr);
                return false;
            }
        }
    }

    return true;
}
