#include <QDir>
#include <QJsonArray>

#include "esmodelement.h"

ESModElement::ESModElement(QNetworkAccessManager *mgr, QObject *parent)
    : QObject(parent),
      title(QStringLiteral("Test sample mod name (Ru,Eng,Spa) [окончен] {99,9 Mb, 1979.01.09}")),
      state(Available),
      progress(100),
      m_NetMgr(mgr),
      m_currDownloadIndex(0)
{
    connect(&m_currDownloadFile, SIGNAL(finished()), this, SLOT(fileClosed()));
    connect(&m_currDownloadFile, SIGNAL(error(QString)), this, SLOT(fileError(QString)));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()));
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)));
    connect(&m_asyncUnzipper, SIGNAL(error(QString)), this, SLOT(unpackError(QString)));
}

ESModElement::ESModElement(State s, int p, QNetworkAccessManager *mgr, QObject *parent)
    : QObject(parent),
      title(QStringLiteral("Test sample mod name (Ru,Eng,Spa) [окончен] {99,9 Mb, 1979.01.09}")),
      state(s),
      progress(p),
      m_NetMgr(mgr),
      m_currDownloadIndex(0)
{
    connect(&m_currDownloadFile, SIGNAL(finished()), this, SLOT(fileClosed()));
    connect(&m_currDownloadFile, SIGNAL(error(QString)), this, SLOT(fileError(QString)));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()));
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)));
    connect(&m_asyncUnzipper, SIGNAL(error(QString)), this, SLOT(unpackError(QString)));
}

ESModElement::~ESModElement()
{
}

QString ESModElement::StateName() const
{
#define DECLARE_STATE_NAME(arg) case arg: return #arg; break;
    switch (state)
    {
    DECLARE_STATE_NAME(Available);
    DECLARE_STATE_NAME(Downloading);
    DECLARE_STATE_NAME(Unpacking);
    DECLARE_STATE_NAME(Failed);
    DECLARE_STATE_NAME(InstalledAvailable);
    DECLARE_STATE_NAME(InstalledHasUpdate);
    DECLARE_STATE_NAME(Installed);
    }
#undef DECLARE_STATE_NAME

    return "Unknown";
}

void ESModElement::Download()
{
    if (state != Available && state != InstalledHasUpdate)
        return;

    if (files.empty() || uri.isEmpty())
        return;

    if (!QDir().mkpath(path))
    {
        state = Failed;
        emit stateChanged();
        return;
    }

    m_currDownloadIndex = 0;
    progress = 0;
    if (!m_currDownloadFile.open(path + files[m_currDownloadIndex]))
    {
        state = Failed;
        emit stateChanged();
        return;
    }

    m_localFiles << path + files[m_currDownloadIndex];

    state = Downloading;
    emit stateChanged();

    QNetworkReply *rep = m_NetMgr->get(QNetworkRequest(QUrl(uri + files[m_currDownloadIndex])));
    connect(rep, SIGNAL(finished()), this, SLOT(fileDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
    connect(rep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    connect(rep, SIGNAL(readyRead()), this, SLOT(readData()));

    connect(&m_currDownloadFile, SIGNAL(error(QString)), rep, SLOT(abort()));
    connect(this, SIGNAL(stopDownload()), rep, SLOT(abort()));
}

void ESModElement::Abort()
{
    emit stopDownload();
    m_currDownloadFile.abort();
    m_asyncUnzipper.abort();
}

void ESModElement::Retry()
{

}

void ESModElement::Update()
{

}

void ESModElement::Delete()
{
    foreach (const QString &fname, m_localFiles)
    {
        QFile::remove(fname);
        QDir().rmpath(QFileInfo(fname).dir().path());
    }

    m_localFiles.clear();
}

void ESModElement::fileDownloaded()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());

    if (rep->error() == QNetworkReply::NoError)
    {
        QByteArray data = rep->readAll();
        m_currDownloadFile.write(data);
    }

    rep->deleteLater();
    m_currDownloadFile.close();
}

void ESModElement::fileError(QString strErr)
{
    Q_UNUSED(strErr)
    state = Failed;
    emit stateChanged();
}

void ESModElement::fileClosed()
{
    // FIXME: Is it necessary?
    m_currDownloadFile.wait();

    if (m_currDownloadFile.aborted())
    {
        Delete();
        state = Available;
        emit stateChanged();
        return;
    }

    if (state != Downloading)
    {
        Delete();
        return;
    }

    ++m_currDownloadIndex;
    if (m_currDownloadIndex >= files.count())
    {
        QStringList zipList;
        foreach (const QString &zipFile, files)
        {
            if (!zipFile.endsWith(".zip", Qt::CaseInsensitive))
                continue;
            zipList << path + zipFile;
        }

        if (m_asyncUnzipper.UnzipList(zipList, path))
        {
            progress = 0;
            state = Unpacking;
        }
        else
        {
            state = Failed;
        }

        emit stateChanged();
        return;
    }

    if (!m_currDownloadFile.open(path + files[m_currDownloadIndex]))
    {
        state = Failed;
        emit stateChanged();
        return;
    }

    m_localFiles << path + files[m_currDownloadIndex];

    QNetworkReply *rep = m_NetMgr->get(QNetworkRequest(QUrl(uri + files[m_currDownloadIndex])));
    connect(rep, SIGNAL(finished()), this, SLOT(fileDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
    connect(rep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    connect(rep, SIGNAL(readyRead()), this, SLOT(readData()));

    connect(&m_currDownloadFile, SIGNAL(error(QString)), rep, SLOT(abort()));
    connect(this, SIGNAL(stopDownload()), rep, SLOT(abort()));
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

    tmp_localFiles << m_asyncUnzipper.getUnpackedFileList();
    m_localFiles.swap(tmp_localFiles);

    if (m_asyncUnzipper.aborted())
    {
        Delete();
        state = Available;
        emit stateChanged();
        return;
    }

    if (state != Unpacking)
    {
        Delete();
        return;
    }

    state = InstalledAvailable;
    emit stateChanged();
}

void ESModElement::downloadError(QNetworkReply::NetworkError err)
{
    if (err == QNetworkReply::OperationCanceledError)
        return;

    state = Failed;
    emit stateChanged();
}

void ESModElement::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    int new_progress = (m_currDownloadIndex * 100 / files.count()) + bytesReceived * 100 / (bytesTotal * files.count());
    if (progress != new_progress)
    {
        progress = new_progress;
        emit stateChanged();
    }
}

void ESModElement::unpackError(QString strErr)
{
    Q_UNUSED(strErr)
    state = Failed;
    emit stateChanged();
}

void ESModElement::unpackProgress(int p)
{
    if (progress != p)
    {
        progress = p;
        emit stateChanged();
    }
}

void ESModElement::readData()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());

    QByteArray data = rep->readAll();
    m_currDownloadFile.write(data);
    // printf("[%s] %d [%lld]\n", __PRETTY_FUNCTION__, data.size(), m_currDownloadFile.bytesToWrite());
}

QJsonObject ESModElement::SerializeToDB()
{
    QJsonArray arr;
    foreach (const QString &fname, m_localFiles)
    {
        arr << fname;
    }

    QJsonObject obj;
    obj["title"] = title;
    obj["files"] = arr;

    return obj;
}

void ESModElement::DeserializeFromDB(QJsonObject obj)
{
    title = obj["title"].toString();

    QJsonArray arr = obj["files"].toArray();
    m_localFiles.clear();
    for (int i = 0; i < arr.size(); ++i)
        m_localFiles << arr[i].toString();

    state = Installed;
}
