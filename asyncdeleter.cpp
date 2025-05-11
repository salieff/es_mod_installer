#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <android/log.h>

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
    qint64 totalElements = 0;
    qint64 remainsElements = 0;
    int progress = 100;
    FolderTreeElement elementsForDelete(this, &totalElements, &remainsElements, &progress);

    for (const auto &fpath: qAsConst(m_localFiles))
        elementsForDelete.addFilePath(fpath);

    elementsForDelete.deleteFolderTree();
}


AsyncDeleter::FolderTreeElement::FolderTreeElement(const AsyncDeleter *deleter,
                                                   qint64 *totalElements,
                                                   qint64 *remainsElements,
                                                   int *progress)
    : m_deleter(deleter)
    , m_totalElements(totalElements)
    , m_remainsElements(remainsElements)
    , m_progress(progress)
{
}

void AsyncDeleter::FolderTreeElement::addFilePath(const QString &path)
{
    addFilePathAsList(QFileInfo(path).dir().path().split('/', Qt::SkipEmptyParts), QFileInfo(path).fileName());
}

void AsyncDeleter::FolderTreeElement::addFilePathAsList(const QStringList &foldersList, const QString &fname)
{
    if (foldersList.empty())
    {
        m_files.push_back(fname);
        ++(*m_totalElements);
        ++(*m_remainsElements);

        if (fname.endsWith(".rpy", Qt::CaseInsensitive))
        {
            m_files.push_back(fname + "c");
            m_files.push_back(fname + "C");

            (*m_totalElements) += 2;
            (*m_remainsElements) += 2;
        }

        return;
    }

    auto it = m_folders.find(foldersList.front());
    if (it == m_folders.end())
    {
        bool b;
        std::tie(it, b) = m_folders.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(foldersList.front()),
                                            std::forward_as_tuple(m_deleter, m_totalElements, m_remainsElements, m_progress));
        ++(*m_totalElements);
        ++(*m_remainsElements);
    }

    it->second.addFilePathAsList(foldersList.mid(1), fname);
}

void AsyncDeleter::FolderTreeElement::deleteFolderTree(const QString &rootFolder)
{
    for (auto &subFolder : m_folders)
    {
        // subFolder.second.deleteFolderTree(rootFolder + "/" + subFolder.first);
        SafAdapter::getCurrentAdapter().DeleteFolder(rootFolder + "/" + subFolder.first);
        __android_log_write(ANDROID_LOG_DEBUG, "deleteFolderTree.DeleteFolder", (rootFolder + "/" + subFolder.first).toLocal8Bit().constData());
        decreaseProgress();
    }

    for (const auto &file : qAsConst(m_files))
    {
        SafAdapter::getCurrentAdapter().DeleteFile(rootFolder + "/" + file);
        __android_log_write(ANDROID_LOG_DEBUG, "deleteFolderTree.DeleteFile", (rootFolder + "/" + file).toLocal8Bit().constData());
        decreaseProgress();
    }
}

void AsyncDeleter::FolderTreeElement::decreaseProgress()
{
    --(*m_remainsElements);
    int currentProgress = (*m_totalElements) > 0 ? (*m_remainsElements) * 100 / (*m_totalElements) : 0;

    if (currentProgress == (*m_progress))
        return;

    (*m_progress) = currentProgress;
    emit m_deleter->progress(*m_progress);
}
