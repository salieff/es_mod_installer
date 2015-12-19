#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "asyncdeleter.h"

AsyncDeleter::AsyncDeleter(QObject *parent)
    : QThread(parent)
{
}

bool AsyncDeleter::deleteFiles(QStringList flist)
{
    m_localFiles = flist;
    start();
    return true;
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
        QDir().rmpath(QFileInfo(fname).dir().path());
    }
}
