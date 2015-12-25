#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QByteArray>
#include <QDir>

#include "asyncdownloader.h"

AsyncDownloader::AsyncDownloader(QObject *parent)
    : QObject(parent),
      m_headersOnly(false),
      m_currFileIndex(0),
      m_wasError(false),
      m_wasAbort(false),
      m_netMgr(this),
      m_file(this),
      m_canOverwrite(false),
      m_alwaysOverwrite(false)
{
#ifndef ANDROID
    // m_netMgr.setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 3128));
#endif

    moveToThread(&m_thread);

    connect(&m_thread, SIGNAL(started()), this, SLOT(download()));
    connect(this, SIGNAL(finished()), &m_thread, SLOT(quit()));
    connect(this, SIGNAL(headersReady()), &m_thread, SLOT(quit()));
}

bool AsyncDownloader::downloadFileList(QString url, QStringList &files, QString destdir, bool headers_only)
{
    m_thread.wait();

    m_headersOnly = headers_only;
    m_url = url;
    m_files = files;
    m_destDir = destdir;
    m_currFileIndex = 0;
    m_wasError = false;
    m_wasAbort = false;
    m_localFiles.clear();
    m_size = 0;
    m_timestamp = 0;
    m_canOverwrite = false;
    m_alwaysOverwrite = false;

    m_thread.start();
    return true;
}

bool AsyncDownloader::wait(unsigned long t)
{
    return m_thread.wait(t);
}

bool AsyncDownloader::aborted()
{
    return m_wasAbort;
}

bool AsyncDownloader::failed()
{
    return m_wasError;
}

void AsyncDownloader::abort()
{
    emit abortDownload();
}

QStringList AsyncDownloader::downloadedFiles()
{
    return m_localFiles;
}

void AsyncDownloader::getHeadersData(double &sz, double &tm)
{
    sz = m_size;
    tm = m_timestamp;
}

void AsyncDownloader::setOverwriteFlags(bool ovrw, bool ovrw_always)
{
    m_overwriteMutex.lock();
    m_canOverwrite = ovrw;
    m_alwaysOverwrite = ovrw_always;
    m_overwriteCondition.wakeAll();
    m_overwriteMutex.unlock();
}

void AsyncDownloader::download()
{
    if (m_files.empty() || m_url.isEmpty())
    {
        if (m_headersOnly)
            emit headersReady();
        else
            emit finished();

        return;
    }

    QNetworkReply *rep = NULL;

    if (m_headersOnly)
    {
        rep = m_netMgr.head(QNetworkRequest(QUrl(m_url + m_files[m_currFileIndex])));
    }
    else
    {
        if (!checkOverwrite(m_destDir + m_files[m_currFileIndex]))
        {
            m_wasAbort = true;
            emit finished();
            return;
        }

        if (!QDir().mkpath(m_destDir))
        {
            m_wasError = true;
            emit finished();
            return;
        }

        m_file.setFileName(m_destDir + m_files[m_currFileIndex]);
        if (!m_file.open(QIODevice::WriteOnly))
        {
            m_wasError = true;
            emit finished();
            return;
        }

        m_localFiles << m_file.fileName();

        rep = m_netMgr.get(QNetworkRequest(QUrl(m_url + m_files[m_currFileIndex])));
        connect(this, SIGNAL(abortDownload()), rep, SLOT(abort()));
        connect(rep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
        connect(rep, SIGNAL(readyRead()), this, SLOT(readData()));
    }

    connect(rep, SIGNAL(finished()), this, SLOT(fileDownloaded()));
}

void AsyncDownloader::fileDownloaded()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    rep->deleteLater();

    if (rep->error() != QNetworkReply::NoError)
    {
        if (rep->error() == QNetworkReply::OperationCanceledError)
            m_wasAbort = true;
        else
            m_wasError = true;

        if (m_headersOnly)
        {
            emit headersReady();
        }
        else
        {
            m_file.close();
            emit finished();
        }

        return;
    }

    if (!m_headersOnly)
    {
        QByteArray data = rep->readAll();
        if (!data.isEmpty() && m_file.write(data) != data.size())
        {
            m_wasError = true;
            emit finished();
            return;
        }

        m_file.close();
    }

    ++m_currFileIndex;
    if (m_currFileIndex >= m_files.count())
    {
        double len = rep->header(QNetworkRequest::ContentLengthHeader).toDouble();
        double tim = rep->header(QNetworkRequest::LastModifiedHeader).toDateTime().toTime_t();

        m_size += len;
        if (tim > m_timestamp)
            m_timestamp = tim;

        if (m_headersOnly)
            emit headersReady();
        else
            emit finished();

        return;
    }

    QNetworkReply *new_rep = NULL;

    if (m_headersOnly)
    {
        new_rep = m_netMgr.head(QNetworkRequest(QUrl(m_url + m_files[m_currFileIndex])));
    }
    else
    {
        if (!checkOverwrite(m_destDir + m_files[m_currFileIndex]))
        {
            m_wasAbort = true;
            emit finished();
            return;
        }

        m_file.setFileName(m_destDir + m_files[m_currFileIndex]);
        if (!m_file.open(QIODevice::WriteOnly))
        {
            m_wasError = true;
            emit finished();
            return;
        }

        m_localFiles << m_file.fileName();

        new_rep = m_netMgr.get(QNetworkRequest(QUrl(m_url + m_files[m_currFileIndex])));
        connect(this, SIGNAL(abortDownload()), new_rep, SLOT(abort()));
        connect(new_rep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
        connect(new_rep, SIGNAL(readyRead()), this, SLOT(readData()));
    }

    connect(new_rep, SIGNAL(finished()), this, SLOT(fileDownloaded()));
}

void AsyncDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_files.empty() || bytesTotal == 0)
        return;

    int p = (m_currFileIndex * 100 / m_files.count()) + bytesReceived * 100 / (bytesTotal * m_files.count());
    emit progress(p);
}

void AsyncDownloader::readData()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    QByteArray data = rep->readAll();
    if (!data.isEmpty() && m_file.write(data) != data.size())
    {
        m_wasError = true;
        emit abortDownload();
    }
}

bool AsyncDownloader::checkOverwrite(QString fname)
{
    if (m_alwaysOverwrite || !QFile::exists(fname))
        return true;

    m_overwriteMutex.lock();
    emit overwriteRequest(fname);
    m_overwriteCondition.wait(&m_overwriteMutex);
    m_overwriteMutex.unlock();

    return m_canOverwrite;
}
