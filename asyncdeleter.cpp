#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <set>

#include "asyncdeleter.h"
#include "safadapter.h"


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
    std::set<QString> localPaths;

    for (const auto &fname: qAsConst(m_localFiles))
    {
        auto topFolder = QFileInfo(fname).dir().path().section('/', 0, 0, QString::SectionSkipEmpty);

        if (topFolder.isEmpty())
        {
            SafAdapter::getCurrentAdapter().DeleteFile(fname);
            if (fname.endsWith(".rpy", Qt::CaseInsensitive))
            {
                SafAdapter::getCurrentAdapter().DeleteFile(fname + "c");
                SafAdapter::getCurrentAdapter().DeleteFile(fname + "C");
            }
        }
        else
        {
            auto [_, success] = localPaths.emplace(topFolder);
            if (success)
                SafAdapter::getCurrentAdapter().DeleteFolder(topFolder);
        }
    }
}
