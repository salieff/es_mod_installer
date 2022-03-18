#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "asyncdeleter.h"

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
        QFile::remove(fname);
        if (fname.endsWith(".rpy", Qt::CaseInsensitive))
        {
            QFile::remove(fname + "c");
            QFile::remove(fname + "C");
        }

        recurseDeleteEmptyDirs(fname);
    }
}

void AsyncDeleter::recurseDeleteEmptyDirs(const QString &childPath)
{
    auto parentDir = QFileInfo(childPath).dir();
    if (!parentDir.isEmpty())
        return;

    if (parentDir.absolutePath() == QDir(m_parentStopFolder).absolutePath())
        return;

    QDir().rmdir(parentDir.absolutePath());
    recurseDeleteEmptyDirs(parentDir.absolutePath());
}
