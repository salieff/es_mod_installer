#include <QDir>
#include <QFile>
#include <QFileInfo>

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
    foreach (const QString &fname, m_localFiles)
    {
        SafAdapter::DeleteFile(fname);
        if (fname.endsWith(".rpy", Qt::CaseInsensitive))
        {
            SafAdapter::DeleteFile(fname + "c");
            SafAdapter::DeleteFile(fname + "C");
        }

        SafAdapter::DeleteEmptyFoldersRecursively(QFileInfo(fname).dir().path(), m_parentStopFolder);
    }
}
