#include <QMessageBox>
#include <QNetworkReply>
#include <QByteArray>
#include <QDir>
#include <QNetworkInterface>
#include <QNetworkProxy>

#if defined(Q_OS_IOS)
    #include "ios_helpers.h"
#elif defined(ANDROID)
    #include <QAndroidJniObject>
#else
    // Desktop
#endif

#include "asyncdownloader.h"

// #define NET_BUFFER_SIZE 1024
#define ES_USER_AGENT "Mozilla/5.0 (Linux; Android 4.4.2; Nexus 5 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.99 Mobile Safari/537.36"
#define RESUME_NOT_FOUND 0
#define RESUME_ALREADY_DONE -1

QNetworkAccessManager * AsyncDownloader::m_networkManager = NULL;
QString AsyncDownloader::m_myMacAddress;
QString AsyncDownloader::m_myUDID;

AsyncDownloader::AsyncDownloader(QObject *parent)
    : QObject(parent),
      m_headersOnly(false),
      m_currFileIndex(0),
      m_progress(0),
      m_wasError(false),
      m_wasAbort(false),
      m_resumeDownloadSize(0),
      m_file(this),
      m_alwaysOverwrite(false)
{
    connect(&m_file, SIGNAL(finished()), this, SLOT(fileWritten()), Qt::QueuedConnection);
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
    m_resumeDownloadSize = 0;
    m_alwaysOverwrite = false;

    if (m_headersOnly)
    {
        m_sizesByName.clear();
        m_timestampsByName.clear();
    }

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

int AsyncDownloader::resumedProgress(QStringList &files, QString destdir)
{
    int ret = 0;

    if (files.empty())
        return ret;

    for (int i = 0; i < files.count(); ++i)
    {
        qint64 refSize = 0;
        qint64 rds = resumeDownloadSize(files[i], destdir, &refSize);
        if (rds == RESUME_ALREADY_DONE)
        {
            ret += 100 / files.count();
            continue;
        }

        if (refSize != 0)
            ret += rds * 100 / (refSize * files.count());
        else
            ret += 100 / files.count();

        break;
    }

    return ret;
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
        new_rep = AsyncDownloader::head(m_url, m_files[m_currFileIndex]);
    }
    else
    {
        // TODO: Overwrite request or resume broken download?
        /*
        if (!checkOverwrite(QDir(m_destDir).filePath(m_files[m_currFileIndex])))
        {
            m_wasAbort = true;
            m_errorString = tr("Aborted by user");
            emit finished();
            return;
        }
        */

        m_resumeDownloadSize = resumeDownloadSize(m_files[m_currFileIndex], m_destDir);
        if (m_resumeDownloadSize == RESUME_ALREADY_DONE)
        {
            m_localFiles << QDir(m_destDir).filePath(m_files[m_currFileIndex]);
            fileWritten();
            return;
        }

        if (!m_file.open(m_destDir, m_files[m_currFileIndex], m_resumeDownloadSize == RESUME_NOT_FOUND ? QIODevice::WriteOnly : QIODevice::Append))
        {
            m_wasError = true;
            m_errorString = m_file.errorString();
            emit finished();
            return;
        }

        m_localFiles << QDir(m_destDir).filePath(m_files[m_currFileIndex]);

        QNetworkRequest new_req(QUrl(m_url).resolved(QUrl(m_files[m_currFileIndex])));
        if (m_resumeDownloadSize != RESUME_NOT_FOUND)
            new_req.setRawHeader(QByteArray("Range"), QString("bytes=%1-").arg(m_resumeDownloadSize).toLatin1());

        new_rep = AsyncDownloader::m_networkManager->get(new_req);
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

    if (m_headersOnly)
    {
        m_sizesByName[m_files[m_currFileIndex]] = len;
        m_timestampsByName[m_files[m_currFileIndex]] = tim;
    }

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

    int p = (m_currFileIndex * 100 / m_files.count()) + (bytesReceived + m_resumeDownloadSize) * 100 / ((bytesTotal + m_resumeDownloadSize) * m_files.count());
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

void AsyncDownloader::createNetworkManager(QObject *parent)
{
    if (m_networkManager == NULL)
    {
        m_networkManager = new QNetworkAccessManager(parent);
#if !defined(ANDROID) && !defined(Q_OS_IOS)
        m_networkManager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 3128));
#endif
    }
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

QString AsyncDownloader::getDeviceUDID()
{
    if (m_myUDID.isEmpty())
    {
#if defined(Q_OS_IOS)
        m_myUDID = ESIOSHelpers::UDID();
#elif defined(ANDROID)
        QAndroidJniObject myID = QAndroidJniObject::fromString("android_id");
        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");
        QAndroidJniObject appctx = activity.callObjectMethod("getApplicationContext","()Landroid/content/Context;");
        QAndroidJniObject contentR = appctx.callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");
        QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod("android/provider/Settings$Secure","getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;",contentR.object<jobject>(), myID.object<jstring>());

        m_myUDID = result.toString();
#else
        // Desktop
        m_myUDID = QString("test0desktop1build");
#endif
    }

    return m_myUDID;
}

QNetworkReply * AsyncDownloader::get(QString url)
{
    return get(QUrl(url));
}

QNetworkReply * AsyncDownloader::get(QString baseUrl, QString fileUrl)
{
    return get(QUrl(baseUrl).resolved(QUrl(fileUrl)));
}

QNetworkReply * AsyncDownloader::get(QUrl url)
{
    QNetworkRequest r;
    r.setUrl(url);
    r.setRawHeader("User-Agent", ES_USER_AGENT);
    r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    return m_networkManager->get(r);
}

QNetworkReply * AsyncDownloader::head(QString url)
{
    return head(QUrl(url));
}

QNetworkReply * AsyncDownloader::head(QString baseUrl, QString fileUrl)
{
    return head(QUrl(baseUrl).resolved(QUrl(fileUrl)));
}

QNetworkReply * AsyncDownloader::head(QUrl url)
{
    QNetworkRequest r;
    r.setUrl(url);
    r.setRawHeader("User-Agent", ES_USER_AGENT);
    r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    return m_networkManager->head(r);
}

qint64 AsyncDownloader::resumeDownloadSize(QString fname, QString destdir, qint64 *refSize)
{
    qint64 sz = QFileInfo(QDir(destdir).filePath(fname)).size(); // If the file does not exist or cannot be fetched, 0 (RESUME_NOT_FOUND) is returned
    if (sz == RESUME_NOT_FOUND)
        return sz;

    std::map<QString, double>::iterator it = m_sizesByName.find(fname);
    if (it != m_sizesByName.end())
    {
        if (refSize != NULL)
            *refSize = it->second;

        if (sz > it->second) // WTF?
            return RESUME_NOT_FOUND;

        if (sz == it->second)
            return RESUME_ALREADY_DONE;
    }
    else
    {
        if (refSize != NULL)
            *refSize = 0;
    }

    return sz;
}
