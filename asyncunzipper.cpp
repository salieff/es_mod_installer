#include <QFileInfo>
#include <QDir>

#include "asyncunzipper.h"

AsyncUnzipper::AsyncUnzipper(QObject *parent)
    : QThread(parent),
      m_totalSize(0),
      m_unpackedSize(0),
      m_progress(0),
      m_abortFlag(false)
{

}

bool AsyncUnzipper::UnzipList(QStringList ziplist, QString destdir)
{
    m_totalSize = 0;
    m_unpackedSize = 0;
    m_progress = 0;
    m_unpackedFiles.clear();
    m_zipList = ziplist;
    m_destDir = destdir;
    m_abortFlag = false;

    start();
    return true;
}

void AsyncUnzipper::abort()
{
    m_abortMutex.lock();
    m_abortFlag = true;
    m_abortMutex.unlock();
}

QStringList AsyncUnzipper::getUnpackedFileList()
{
    return m_unpackedFiles;
}

void AsyncUnzipper::run()
{
    if (!calculateTotalSize())
    {
        emit error("Can't calculate total size");
        return;
    }

    foreach (const QString &zipFile, m_zipList)
    {
        if (!unpackZip(zipFile))
        {
            emit error("Can't unpack " + zipFile);
            break;
        }

        if (aborted())
            break;
    }
}

bool AsyncUnzipper::calculateTotalSize()
{
    foreach (const QString &zipFile, m_zipList)
    {
        unzFile ufd = unzOpen(zipFile.toLocal8Bit());
        if (ufd == NULL)
            return false;

        if (unzGoToFirstFile(ufd) != UNZ_OK)
            return false;

        do
        {
            unz_file_info finfo;
            char fname[1025];
            if (unzGetCurrentFileInfo(ufd, &finfo, fname, sizeof(fname) - 1, NULL, 0, NULL, 0) != UNZ_OK)
                return false;

            fname[sizeof(fname) - 1] = 0;

            // We don't need in directories records
            if (QString(fname).endsWith("/") && finfo.compressed_size == 0 && finfo.uncompressed_size == 0 && finfo.crc == 0)
                continue;

            m_totalSize += finfo.uncompressed_size;

            // printf("[%s %lld] %s (%lu, %lu)\n", zipFile.toLocal8Bit().data(), m_totalSize, fname, finfo.compressed_size, finfo.uncompressed_size);
        } while (unzGoToNextFile(ufd) == UNZ_OK);

        if (unzClose(ufd) != UNZ_OK)
            return false;
    }

    return true;
}

bool AsyncUnzipper::unpackZip(QString zipFile)
{
    unzFile ufd = unzOpen(zipFile.toLocal8Bit());
    if (ufd == NULL)
        return false;

    if (unzGoToFirstFile(ufd) != UNZ_OK)
        return false;

    do
    {
        unz_file_info finfo;
        char fname[1025];
        if (unzGetCurrentFileInfo(ufd, &finfo, fname, sizeof(fname) - 1, NULL, 0, NULL, 0) != UNZ_OK)
            return false;

        fname[sizeof(fname) - 1] = 0;

        // We don't need in directories records
        if (QString(fname).endsWith("/") && finfo.compressed_size == 0 && finfo.uncompressed_size == 0 && finfo.crc == 0)
            continue;

        if (unzOpenCurrentFile(ufd) != UNZ_OK)
            return false;

        if (!saveCurrentUnpFile(ufd, m_destDir + fname))
            return false;

        if (unzCloseCurrentFile(ufd) != UNZ_OK)
            return false;

        if (aborted())
            break;

    } while (unzGoToNextFile(ufd) == UNZ_OK);

    if (unzClose(ufd) != UNZ_OK)
        return false;

    return true;
}

bool AsyncUnzipper::saveCurrentUnpFile(unzFile ufd, QString fname)
{
    if (!QDir().mkpath(QFileInfo(fname).dir().path()))
        return false;

    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    m_unpackedFiles << fname;

    int unzRet = 0;
    char buf[512 * 1024];

    while((unzRet = unzReadCurrentFile(ufd, buf, sizeof(buf))) > 0)
    {
        if (!file.write(buf, unzRet))
        {
            unzRet = -1;
            emit error(file.errorString());
            break;
        }
        else
        {
            m_unpackedSize += unzRet;
            int new_progress = m_unpackedSize * 100 / m_totalSize;
            if (m_progress != new_progress)
            {
                m_progress = new_progress;
                emit progress(m_progress);
            }
        }

        if (aborted())
            break;
    }

    file.close();
    return (unzRet >= 0);
}

bool AsyncUnzipper::aborted()
{
    bool ret;

    m_abortMutex.lock();
    ret = m_abortFlag;
    m_abortMutex.unlock();

    return ret;
}
