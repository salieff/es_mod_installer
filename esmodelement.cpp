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
      m_localSize(0),
      m_localTimestamp(0)
{
    connect(&m_asyncDownloader, SIGNAL(progress(int)), this, SLOT(downloadProgress(int)));
    connect(&m_asyncDownloader, SIGNAL(finished()), this, SLOT(filesDownloaded()));
    connect(&m_asyncDownloader, SIGNAL(headersReady()), this, SLOT(headersReceived()));
    connect(this, SIGNAL(abortProcessing()), &m_asyncDownloader, SLOT(abort()));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()));
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)));
    connect(this, SIGNAL(abortProcessing()), &m_asyncUnzipper, SLOT(abort()));
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
    if (state != Available && state != InstalledHasUpdate)
        return;

    if (files.empty() || uri.isEmpty())
        return;

    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    if (!m_asyncDownloader.downloadFileList(uri, files, path))
        state = Failed;
    else
        state = Downloading;

    progress = 0;
    emit stateChanged();
}

void ESModElement::Abort()
{
    emit abortProcessing();
}

void ESModElement::Update()
{
    Delete();
    Download();
}

void ESModElement::Delete()
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

    m_localFiles.clear();
    m_localSize = 0;
    m_localTimestamp = 0;
}

void ESModElement::RequestHeaders()
{
    if (files.empty() || uri.isEmpty())
    {
        state = Installed;
        emit stateChanged();
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
            state = Available;
        else
            state = InstalledAvailable;

        emit stateChanged();
    }

    m_asyncDownloader.getHeadersData(size, timestamp);
    if (m_localFiles.empty())
    {
        state = Available;
    }
    else
    {
        if (timestamp > m_localTimestamp)
            state = InstalledHasUpdate;
        else
            state = InstalledAvailable;
    }
    emit stateChanged();
}

void ESModElement::zipListUnpacked()
{
    // FIXME: Is it necessary?
    m_asyncUnzipper.wait();

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
        if (m_asyncUnzipper.aborted())
        {
            state = Available;
            emit stateChanged();
        }
        if (m_asyncUnzipper.failed())
        {
            state = Failed;
            emit stateChanged();
        }
        return;
    }

    m_localTimestamp = timestamp;
    m_localSize = size;

    state = InstalledAvailable;
    emit stateChanged();
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
    m_localFiles = m_asyncDownloader.downloadedFiles();

    if (state != Downloading || m_asyncDownloader.aborted() || m_asyncDownloader.failed())
    {
        Delete();
        if (m_asyncDownloader.aborted())
        {
            state = Available;
            emit stateChanged();
        }
        if (m_asyncDownloader.failed())
        {
            state = Failed;
            emit stateChanged();
        }
        return;
    }

    QStringList zipList;
    foreach (const QString &zipFile, files)
        if (zipFile.endsWith(".zip", Qt::CaseInsensitive))
            zipList << path + zipFile;

    if (m_asyncUnzipper.unzipList(zipList, path))
    {
        progress = 0;
        state = Unpacking;
    }
    else
    {
        state = Failed;
    }

    emit stateChanged();
}

void ESModElement::downloadProgress(int p)
{
    if (progress == p)
        return;

    progress = p;
    emit stateChanged();
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
