#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <set>
#include <algorithm>

#include "asyncdeleter.h"
#include "safadapter.h"


AsyncDeleter::AsyncDeleter(QObject *parent, QString stopFolder)
    : QThread(parent)
    , m_parentStopFolder(stopFolder)
{
}

bool AsyncDeleter::deleteFiles(QStringList flist)
{
    m_localFiles = flist;
    start();
    return true;
}

void AsyncDeleter::setStopFolder(QString stopFolder)
{
    m_parentStopFolder = stopFolder;
}

void AsyncDeleter::run()
{
    std::set<QString> localPaths;

    foreach (const QString &fname, m_localFiles)
    {
        SafAdapter::DeleteFile(fname);
        if (fname.endsWith(".rpy", Qt::CaseInsensitive))
        {
            SafAdapter::DeleteFile(fname + "c");
            SafAdapter::DeleteFile(fname + "C");
        }

        localPaths.insert(QFileInfo(fname).dir().path());
    }

    std::vector<QString> optimizedPaths;

    for (auto const &path : localPaths)
    {
        // Если в localPaths есть более длинный путь, поглощающий текущий, то не добавляем текущий в optimizedPaths
        if (std::find_if(
                    localPaths.begin(),
                    localPaths.end(),
                    [&path](const QString &localPath){ return localPath.length() > path.length() && localPath.startsWith(path); }
                    ) != localPaths.end())
            continue;

        optimizedPaths.push_back(path);
    }

    for (auto const &path : optimizedPaths)
        SafAdapter::DeleteEmptyFoldersRecursively(path, m_parentStopFolder);
}
