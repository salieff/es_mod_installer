#include <QFileInfo>
#include <QDir>

#include "asyncunzipper.h"
#include "safadapter.h"

#define UNPACK_BUFFER_SIZE (4*1024*1024)

AsyncUnzipper::AsyncUnzipper(QObject *parent)
    : QThread(parent),
      m_totalSize(0),
      m_unpackedSize(0),
      m_progress(0),
      m_abortFlag(false),
      m_failedFlag(false)
{
}

bool AsyncUnzipper::unzip(QString zipfile)
{
    m_totalSize = 0;
    m_unpackedSize = 0;
    m_progress = 0;
    m_unpackedFiles.clear();
    m_zipFile = zipfile;
    m_abortFlag = false;
    m_failedFlag = false;
    m_errorString.clear();
    m_topFolders.clear();
    m_topFiles.clear();
    m_foldersToCreate.clear();

    start();
    return true;
}

bool AsyncUnzipper::aborted()
{
    return m_abortFlag;
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
    m_errorString = tr("Aborted by user");
    m_abortFlag = true;
}

QStringList AsyncUnzipper::unpackedFiles()
{
    return m_unpackedFiles;
}

void AsyncUnzipper::run()
{
    if (!unpackZip(ESTIMATE))
    {
        m_failedFlag = true;
        return;
    }

    deletePreviousInstallation();

    if (!m_foldersToCreate.createFolderTree(m_errorString))
    {
        m_failedFlag = true;
        return;
    }

    m_unpackZipBuffer = new char[UNPACK_BUFFER_SIZE];

    if (!unpackZip(UNPACK))
        m_failedFlag = true;

    delete[] m_unpackZipBuffer;
    m_unpackZipBuffer = nullptr;
}

bool AsyncUnzipper::unpackZip(bool estimateOnly)
{
    unzFile ufd = unzOpen2(m_zipFile.toLocal8Bit(), &SafAdapter::MiniZipFileAPI);
    if (ufd == NULL)
    {
        m_errorString = tr("Can't open zip file ") + m_zipFile;
        return false;
    }

    if (unzGoToFirstFile(ufd) != UNZ_OK)
    {
        m_errorString = tr("Can't go to fist entry in zip file ") + m_zipFile;
        return false;
    }

    do
    {
        unz_file_info finfo;
        char fnameBuff[1025];
        if (unzGetCurrentFileInfo(ufd, &finfo, fnameBuff, sizeof(fnameBuff) - 1, NULL, 0, NULL, 0) != UNZ_OK)
        {
            m_errorString = tr("Can't get current entry info in zip file ") + m_zipFile;
            return false;
        }

        fnameBuff[sizeof(fnameBuff) - 1] = 0;
        QString fname(fnameBuff);

        // We don't need in directories records
        if (fname.endsWith("/"))
            continue;

        fname.remove(QRegExp("^/*")); // To avoid absolute paths

        if (estimateOnly)
        {
            m_totalSize += finfo.uncompressed_size;

            auto foldersList = QFileInfo(fname).dir().path().split('/', Qt::SkipEmptyParts);

            if (foldersList.empty())
            {
                m_topFiles.append(fname);
                if (fname.endsWith(".rpy", Qt::CaseInsensitive))
                {
                    m_topFiles.append(fname + "c");
                    m_topFiles.append(fname + "C");
                }
            }
            else
            {
                m_topFolders.insert(foldersList.front());
                m_foldersToCreate.addFoldersList(foldersList);
            }
        }
        else
        {
            if (unzOpenCurrentFile(ufd) != UNZ_OK)
            {
                m_errorString = tr("Can't open current entry ") + fname + tr(" in zip file ") + m_zipFile;
                return false;
            }

            if (!saveCurrentUnpFile(ufd, fname))
                return false;

            if (unzCloseCurrentFile(ufd) != UNZ_OK)
            {
                m_errorString = tr("Can't close current entry ") + fname + tr(" in zip file ") + m_zipFile;
                return false;
            }
        }

        if (aborted())
            break;
    } while (unzGoToNextFile(ufd) == UNZ_OK);

    if (unzClose(ufd) != UNZ_OK)
    {
        m_errorString = tr("Can't close zip file ") + m_zipFile;
        return false;
    }

    return true;
}

bool AsyncUnzipper::saveCurrentUnpFile(unzFile ufd, QString fname)
{
    QFile file;
    if (!SafAdapter::getCurrentAdapter().CreateQFile(file, fname, QIODevice::WriteOnly | QIODevice::Truncate))
    {
        m_errorString = tr("Can't create file ") + fname + " : " + file.errorString();
        return false;
    }

    m_unpackedFiles << fname;

    int unzRet = 0;
    while((unzRet = unzReadCurrentFile(ufd, m_unpackZipBuffer, UNPACK_BUFFER_SIZE)) > 0)
    {
        if (!file.write(m_unpackZipBuffer, unzRet))
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

void AsyncUnzipper::deletePreviousInstallation()
{
    for (const auto &file : qAsConst(m_topFiles))
        SafAdapter::getCurrentAdapter().DeleteFile(file);

    for (const auto &folder : qAsConst(m_topFolders))
        SafAdapter::getCurrentAdapter().DeleteFolder(folder);
}


void AsyncUnzipper::FolderTreeElement::clear()
{
    children.clear();
}

void AsyncUnzipper::FolderTreeElement::addFoldersList(const QStringList &foldersList)
{
    if (!foldersList.empty())
        children[foldersList.front()].addFoldersList(foldersList.mid(1));
}

bool AsyncUnzipper::FolderTreeElement::createFolderTree(QString &errorString, const QString &rootFolder) const
{
    for (const auto &el : qAsConst(children))
    {
        if (!SafAdapter::getCurrentAdapter().CreateFolder(rootFolder, el.first))
        {
            errorString = tr("Can't create folder ") + rootFolder + "/" + el.first;
            return false;
        }

        if (!el.second.createFolderTree(errorString, rootFolder + "/" + el.first))
            return false;
    }

    return true;
}
