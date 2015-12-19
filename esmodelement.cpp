#include <QDir>
#include <QJsonArray>

#include "esmodelement.h"

ESModElement::ESModElement(QObject *parent, State s, int p)
    : QObject(parent),
      title(QStringLiteral("Test sample mod name (Ru,Eng,Spa) [окончен] {99,9 Mb, 1979.01.09}")),
      state(s),
      progress(p),
      size(0),
      timestamp(0),
      guiblocked(1),
      m_localSize(0),
      m_localTimestamp(0)
{
    connect(&m_asyncDownloader, SIGNAL(progress(int)), this, SLOT(downloadProgress(int)));
    connect(&m_asyncDownloader, SIGNAL(finished()), this, SLOT(filesDownloaded()));
    connect(&m_asyncDownloader, SIGNAL(headersReady()), this, SLOT(headersReceived()));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()));
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)));

    connect(&m_asyncDeleter, SIGNAL(finished()), this, SLOT(filesDeleted()));
}

QString ESModElement::StateName() const
{
#define DECLARE_STATE_NAME(arg) case arg: return #arg; break;
    switch (state)
    {
    DECLARE_STATE_NAME(Unknown);
    DECLARE_STATE_NAME(Available);
    DECLARE_STATE_NAME(Downloading);
    DECLARE_STATE_NAME(Unpacking);
    DECLARE_STATE_NAME(Failed);
    DECLARE_STATE_NAME(InstalledAvailable);
    DECLARE_STATE_NAME(InstalledHasUpdate);
    DECLARE_STATE_NAME(Installed);
    }
#undef DECLARE_STATE_NAME

    return "###???###";
}

void ESModElement::Download()
{
    if (state != Available && state != InstalledHasUpdate && state != Failed)
        return;

    if (files.empty() || uri.isEmpty())
        return;

    blockGui();

    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    connect(this, SIGNAL(abortProcessing()), &m_asyncDownloader, SLOT(abort()));

    if (!m_asyncDownloader.downloadFileList(uri, files, path))
        changeState(Failed);
    else
        changeState(Downloading);
}

void ESModElement::Abort()
{
    blockGui();
    emit abortProcessing();
}

void ESModElement::Update()
{
    blockGui();
    m_asyncDeleter.wait();
    m_asyncDeleter.deleteFiles(m_localFiles);
}

void ESModElement::Delete()
{
    blockGui(2);
    m_asyncDeleter.wait();
    m_asyncDeleter.deleteFiles(m_localFiles);
}

void ESModElement::RequestHeaders()
{
    if (files.empty() || uri.isEmpty())
    {
        changeState(Installed);
        return;
    }

    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    m_asyncDownloader.downloadFileList(uri, files, path, true);
}

void ESModElement::headersReceived()
{
    if (state != Unknown)
        return;

    if (m_asyncDownloader.aborted() || m_asyncDownloader.failed())
    {
        if (m_localFiles.empty())
            changeState(Available);
        else
            changeState(InstalledAvailable);
    }

    m_asyncDownloader.getHeadersData(size, timestamp);
    if (m_localFiles.empty())
    {
        changeState(Available);
    }
    else
    {
        if (timestamp > m_localTimestamp)
            changeState(InstalledHasUpdate);
        else
            changeState(InstalledAvailable);
    }
}

void ESModElement::zipListUnpacked()
{
    disconnect(this, SIGNAL(abortProcessing()), &m_asyncUnzipper, SLOT(abort()));

    QStringList tmp_localFiles;
    for (int i = 0; i < m_localFiles.count(); ++i)
    {
        if (m_localFiles[i].endsWith(".zip", Qt::CaseInsensitive) && QFile::remove(m_localFiles[i]))
            continue;

        tmp_localFiles << m_localFiles[i];
    }

    tmp_localFiles << m_asyncUnzipper.unpackedFiles();
    m_localFiles.swap(tmp_localFiles);

    if (state != Unpacking || m_asyncUnzipper.aborted() || m_asyncUnzipper.failed())
    {
        Delete();
        return;
    }

    m_localTimestamp = timestamp;
    m_localSize = size;

    changeState(InstalledAvailable);
    emit saveMe();
}

void ESModElement::unpackProgress(int p)
{
    if (progress == p)
        return;

    progress = p;
    emit stateChanged();
}

void ESModElement::filesDownloaded()
{
    disconnect(this, SIGNAL(abortProcessing()), &m_asyncDownloader, SLOT(abort()));

    m_localFiles = m_asyncDownloader.downloadedFiles();

    if (state != Downloading || m_asyncDownloader.aborted() || m_asyncDownloader.failed())
    {
        Delete();
        return;
    }

    QStringList zipList;
    foreach (const QString &zipFile, files)
        if (zipFile.endsWith(".zip", Qt::CaseInsensitive))
            zipList << path + zipFile;

    connect(this, SIGNAL(abortProcessing()), &m_asyncUnzipper, SLOT(abort()));

    if (m_asyncUnzipper.unzipList(zipList, path))
        changeState(Unpacking);
    else
        changeState(Failed);
}

void ESModElement::downloadProgress(int p)
{
    if (progress == p)
        return;

    progress = p;
    emit stateChanged();
}

void ESModElement::filesDeleted()
{
    m_localFiles.clear();
    m_localSize = 0;
    m_localTimestamp = 0;

    switch(state)
    {
    case Downloading :
        if (m_asyncDownloader.aborted())
            changeState(Available);
        if (m_asyncDownloader.failed())
            changeState(Failed);
        break;

    case Unpacking :
        if (m_asyncUnzipper.aborted())
            changeState(Available);
        if (m_asyncUnzipper.failed())
            changeState(Failed);
        break;

    case InstalledAvailable :
        changeState(Available);
        emit saveMe();
        break;

    case InstalledHasUpdate :
        if (guiblocked == 1)
        {
            state = Available;
            emit saveMe();
            Download();
        }
        if (guiblocked == 2)
        {
            changeState(Available);
            emit saveMe();
        }
        break;

    case Installed :
        state = Unknown;
        deleteLater();
        emit removeMe();
        emit saveMe();
        break;
    }
}

QJsonObject ESModElement::SerializeToDB()
{
    QJsonArray arr;
    foreach (const QString &fname, m_localFiles)
        arr << fname;

    QJsonObject obj;
    obj["title"] = title;
    obj["files"] = arr;
    obj["size"] = m_localSize;
    obj["timestamp"] = m_localTimestamp;

    return obj;
}

void ESModElement::DeserializeFromDB(QJsonObject obj)
{
    title = obj["title"].toString();
    m_localSize = obj["size"].toDouble();
    m_localTimestamp = obj["timestamp"].toDouble();

    QJsonArray arr = obj["files"].toArray();
    m_localFiles.clear();
    for (int i = 0; i < arr.size(); ++i)
        m_localFiles << arr[i].toString();
}

void ESModElement::blockGui(int b)
{
    guiblocked = b;
    emit stateChanged();
}

void ESModElement::changeState(State s)
{
    if (s == Downloading || s == Unpacking)
        progress = 0;
    else
        progress = 100;

    guiblocked = 0;
    state = s;
    emit stateChanged();
}
