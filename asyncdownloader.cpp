#include <QMessageBox>
#include <QNetworkReply>
#include <QByteArray>
#include <QDir>
#include <QNetworkInterface>

#include "asyncdownloader.h"

// #define NET_BUFFER_SIZE 1024

QNetworkAccessManager * AsyncDownloader::NetworkManager = NULL;
QString AsyncDownloader::m_myMacAddress;

AsyncDownloader::AsyncDownloader(QObject *parent)
    : QObject(parent),
      m_headersOnly(false),
      m_currFileIndex(0),
      m_progress(0),
      m_wasError(false),
      m_wasAbort(false),
      m_file(this),
      m_alwaysOverwrite(false)
{
    connect(&m_file, SIGNAL(finished()), this, SLOT(fileWritten()));
}

AsyncDownloader::~AsyncDownloader()
{
}

bool AsyncDownloader::downloadFileList(QString url, QStringList &files, QString destdir, bool headers_only)
{
    m_file.wait();

    m_headersOnly = headers_only;
    m_url = url;
    m_files = files;
    m_destDir = destdir;
    m_currFileIndex = -1;
    m_progress = 0;
    m_wasError = false;
    m_wasAbort = false;
    m_errorString.clear();
    m_localFiles.clear();
    m_size = 0;
    m_timestamp = 0;
    m_alwaysOverwrite = false;

    fileWritten();
    return true;
}

bool AsyncDownloader::wait(unsigned long t)
{
    return m_file.wait(t);
}

bool AsyncDownloader::aborted()
{
    return m_wasAbort;
}

bool AsyncDownloader::failed()
{
    return m_wasError;
}

QString AsyncDownloader::errorString()
{
    return m_errorString;
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

void AsyncDownloader::fileWritten()
{
    m_wasAbort = m_wasAbort || m_file.aborted();
    m_wasError = m_wasError || m_file.failed();
    if (m_errorString.isEmpty())
        m_errorString = m_file.errorString();

    ++m_currFileIndex;
    if (m_currFileIndex >= m_files.count() || m_wasAbort || m_wasError)
    {
        if (m_headersOnly)
            emit headersReady();
        else
            emit finished();

        return;
    }

    QNetworkReply *new_rep = NULL;

    if (m_headersOnly)
    {
        new_rep = AsyncDownloader::NetworkManager->head(QNetworkRequest(QUrl(m_url + m_files[m_currFileIndex])));
    }
    else
    {
        if (!checkOverwrite(m_destDir + m_files[m_currFileIndex]))
        {
            m_wasAbort = true;
            m_errorString = tr("Aborted by user");
            emit finished();
            return;
        }

        if (!m_file.open(m_destDir, m_files[m_currFileIndex]))
        {
            m_wasError = true;
            m_errorString = m_file.errorString();
            emit finished();
            return;
        }

        m_localFiles << m_destDir + m_files[m_currFileIndex];

        new_rep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(m_url + m_files[m_currFileIndex])));
        connect(this, SIGNAL(abortDownload()), new_rep, SLOT(abort()));
        connect(new_rep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
        connect(new_rep, SIGNAL(readyRead()), this, SLOT(readData()));
    }

    //    new_rep->setReadBufferSize(NET_BUFFER_SIZE);
    connect(new_rep, SIGNAL(finished()), this, SLOT(fileDownloaded()));
}

void AsyncDownloader::fileDownloaded()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    rep->deleteLater();

    double len = rep->header(QNetworkRequest::ContentLengthHeader).toDouble();
    double tim = rep->header(QNetworkRequest::LastModifiedHeader).toDateTime().toTime_t();

    m_size += len;
    if (tim > m_timestamp)
        m_timestamp = tim;

    if (rep->error() == QNetworkReply::OperationCanceledError)
        m_wasAbort = true;
    else if (rep->error() != QNetworkReply::NoError)
        m_wasError = true;

    if (m_wasError || m_wasAbort)
        m_errorString = rep->errorString();

    if (!m_headersOnly)
    {
        if (!(m_wasError || m_wasAbort))
            m_file.write(rep);

        m_file.close();
    }
    else
    {
        fileWritten();
    }
}

void AsyncDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_files.empty() || bytesTotal == 0)
        return;

    int p = (m_currFileIndex * 100 / m_files.count()) + bytesReceived * 100 / (bytesTotal * m_files.count());
    if (m_progress != p)
    {
        m_progress = p;
        emit progress(m_progress);
    }
}

void AsyncDownloader::readData()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    m_file.write(rep);
}

bool AsyncDownloader::checkOverwrite(QString fname)
{
    if (m_alwaysOverwrite || !QFile::exists(fname))
        return true;

    QMessageBox::StandardButton b = QMessageBox::warning(NULL, tr("Risk of overwriting"), \
                                                         tr("File %1 already exists, do you want to overwrite it?").arg(fname), \
                                                         QMessageBox::Cancel | QMessageBox::Yes | QMessageBox::YesToAll, \
                                                         QMessageBox::Cancel);

    m_alwaysOverwrite = (b == QMessageBox::YesToAll);
    return (b == QMessageBox::YesToAll || b == QMessageBox::Yes);
}

QString AsyncDownloader::getMacAddress()
{
    if (m_myMacAddress.isEmpty())
    {
        QList<QNetworkInterface> allIfaces = QNetworkInterface::allInterfaces();
        foreach (QNetworkInterface i, allIfaces)
        {
            if (!i.isValid())
                continue;

            if (i.flags() & QNetworkInterface::IsLoopBack)
                continue;

            if (!(i.flags() & QNetworkInterface::IsUp))
                continue;

            if (!(i.flags() & QNetworkInterface::IsRunning))
                continue;

            m_myMacAddress = i.hardwareAddress();
            break;
        }
    }

    return m_myMacAddress;
}
