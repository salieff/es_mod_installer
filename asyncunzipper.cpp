#include <QFileInfo>
#include <QDir>

#include "asyncunzipper.h"
#include "safadapter.h"
#include "modpaths.h"


#define UNPACK_BUFFER_SIZE (16*1024)

AsyncUnzipper::AsyncUnzipper(QObject *parent, QString stopFolder)
    : QThread(parent),
      m_totalSize(0),
      m_unpackedSize(0),
      m_progress(0),
      m_abortFlag(false),
      m_failedFlag(false),
      m_canOverwrite(false),
      m_alwaysOverwrite(false),
      m_parentStopFolder(stopFolder)
{
}

bool AsyncUnzipper::unzipList(QStringList ziplist, QString destdir)
{
    m_totalSize = 0;
    m_unpackedSize = 0;
    m_progress = 0;
    m_unpackedFiles.clear();
    m_zipList = ziplist;
    m_destDir = destdir;
    m_abortFlag = false;
    m_failedFlag = false;
    m_errorString.clear();
    m_canOverwrite = false;
    m_alwaysOverwrite = false;

    start();
    return true;
}

bool AsyncUnzipper::aborted()
{
    bool ret;

    m_abortMutex.lock();
    ret = m_abortFlag;
    m_abortMutex.unlock();

    return ret;
}

bool AsyncUnzipper::failed()
{
    return m_failedFlag;
}

QString AsyncUnzipper::errorString()
{
    return m_errorString;
}

void AsyncUnzipper::abort()
{
    m_abortMutex.lock();
    m_errorString = tr("Aborted by user");
    m_abortFlag = true;
    m_abortMutex.unlock();
}

QStringList AsyncUnzipper::unpackedFiles()
{
    return m_unpackedFiles;
}

void AsyncUnzipper::setOverwriteFlags(bool ovrw, bool ovrw_always)
{
    m_overwriteMutex.lock();
    m_canOverwrite = ovrw;
    m_alwaysOverwrite = ovrw_always;
    m_overwriteCondition.wakeAll();
    m_overwriteMutex.unlock();
}

void AsyncUnzipper::setStopFolder(QString stopFolder)
{
    m_parentStopFolder = stopFolder;
}

void AsyncUnzipper::run()
{
    if (!calculateTotalSize())
    {
        m_failedFlag = true;
        return;
    }

    for (const QString &zipFile: m_zipList)
    {
        if (!unpackZip(zipFile))
        {
            m_failedFlag = true;
            break;
        }

        if (!SafAdapter::getCurrentAdapter().DeleteFile(zipFile))
        {
            m_failedFlag = true;
            m_errorString = tr("Can't delete zipfile ") + zipFile;
            break;
        }

        SafAdapter::getCurrentAdapter().DeleteEmptyFoldersRecursively(QFileInfo(zipFile).dir().path(), m_parentStopFolder);

        if (aborted())
            break;
    }
}

bool AsyncUnzipper::calculateTotalSize()
{
    m_totalSize = 0;
    foreach (const QString &zipFile, m_zipList)
        if (!unpackZip(zipFile, true))
            return false;

    return true;
}

bool AsyncUnzipper::unpackZip(QString zipFile, bool calcSizeOnly)
{
    unzFile ufd = unzOpen2(zipFile.toLocal8Bit(), &SafAdapter::MiniZipFileAPI);
    if (ufd == NULL)
    {
        m_errorString = tr("Can't open zip file ") + zipFile;
        return false;
    }

    if (unzGoToFirstFile(ufd) != UNZ_OK)
    {
        m_errorString = tr("Can't go to fist entry in zip file ") + zipFile;
        return false;
    }

    do
    {
        unz_file_info finfo;
        char fnameBuff[1025];
        if (unzGetCurrentFileInfo(ufd, &finfo, fnameBuff, sizeof(fnameBuff) - 1, NULL, 0, NULL, 0) != UNZ_OK)
        {
            m_errorString = tr("Can't get current entry info in zip file ") + zipFile;
            return false;
        }

        fnameBuff[sizeof(fnameBuff) - 1] = 0;
        QString fname(fnameBuff);

        // We don't need in directories records
        if (fname.endsWith("/"))
            continue;

        fname.remove(QRegExp("^/*")); // To avoid absolute paths

        if (calcSizeOnly)
        {
            m_totalSize += finfo.uncompressed_size;
        }
        else
        {
            if (unzOpenCurrentFile(ufd) != UNZ_OK)
            {
                m_errorString = tr("Can't open current entry ") + fname + tr(" in zip file ") + zipFile;
                return false;
            }

            if (!saveCurrentUnpFile(ufd, QDir(m_destDir).filePath(fname)))
                return false;

            if (unzCloseCurrentFile(ufd) != UNZ_OK)
            {
                m_errorString = tr("Can't close current entry ") + fname + tr(" in zip file ") + zipFile;
                return false;
            }
        }

        if (aborted())
            break;
    } while (unzGoToNextFile(ufd) == UNZ_OK);

    if (unzClose(ufd) != UNZ_OK)
    {
        m_errorString = tr("Can't close zip file ") + zipFile;
        return false;
    }

    return true;
}

bool AsyncUnzipper::saveCurrentUnpFile(unzFile ufd, QString fname)
{
    if (!checkOverwrite(fname))
    {
        m_errorString = tr("Aborted by user");
        m_abortFlag = true;
        return true;
    }

    QFile file;
    if (!SafAdapter::getCurrentAdapter().CreateQFile(file, fname, QIODevice::WriteOnly | QIODevice::Truncate, SafAdapter::CREATE_FOLDERS))
    {
        m_errorString = tr("Can't create file ") + fname + " : " + file.errorString();
        return false;
    }

    m_unpackedFiles << fname;

    int unzRet = 0;
    char buf[UNPACK_BUFFER_SIZE];

    while((unzRet = unzReadCurrentFile(ufd, buf, sizeof(buf))) > 0)
    {
        if (!file.write(buf, unzRet))
        {
            unzRet = -1;
            m_errorString = file.errorString();
            m_failedFlag = true;
            break;
        }
        else
        {
            m_unpackedSize += unzRet;
            if (m_totalSize > 0)
            {
                int new_progress = m_unpackedSize * 100 / m_totalSize;
                if (m_progress != new_progress)
                {
                    m_progress = new_progress;
                    emit progress(m_progress);
                }
            }
        }

        if (aborted())
            break;
    }

    file.close();
    return (unzRet >= 0);
}

bool AsyncUnzipper::checkOverwrite(QString fname)
{
    if (m_alwaysOverwrite || !SafAdapter::getCurrentAdapter().FileExists(fname))
        return true;

    m_overwriteMutex.lock();
    emit overwriteRequest(fname);
    m_overwriteCondition.wait(&m_overwriteMutex);
    m_overwriteMutex.unlock();

    return m_canOverwrite;
}
