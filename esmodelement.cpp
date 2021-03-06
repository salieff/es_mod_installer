#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QTimer>

#include "esmodelement.h"
#include "statisticsmanager.h"

#if defined(Q_OS_IOS)
    #define MY_PLATFORM "ios"
#elif defined(ANDROID)
    #define MY_PLATFORM "android"
#else
//    #define MY_PLATFORM "ios"
    #define MY_PLATFORM "android"
#endif

ESModElement::ESModElement(QString au, QString ap, QObject *parent, State st, int pr)
    : QObject(parent),
      id(-1),
      title(QStringLiteral("Test sample mod name")),
      langs(QStringList() << "Ru" << "En" <<"Spa"),
      status("окончен"),
      state(st),
      progress(pr),
      size(0),
      timestamp(0),
      mylikemark(LikeMarkNotFound),
      likemarkscount(-1),
      dislikemarkscount(-1),
      insttotal(-1),
      instactive(-1),
      insttotalmonth(-1),
      instactivemonth(-1),
      insttotalweek(-1),
      instactiveweek(-1),
      lifetimeavg(-1),
      lifetimemax(-1),
      guiblocked(ByUnknown),
      m_localSize(0),
      m_localTimestamp(0),
      m_modelIndex(-1),
      m_uri(au),
      m_path(ap),
      m_failedDownloadsCount(0)
{
    connect(&m_asyncDownloader, SIGNAL(progress(int)), this, SLOT(downloadProgress(int)));
    connect(&m_asyncDownloader, SIGNAL(finished()), this, SLOT(filesDownloaded()));
    connect(&m_asyncDownloader, SIGNAL(headersReady()), this, SLOT(headersReceived()));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()), Qt::QueuedConnection);
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)), Qt::QueuedConnection);
    connect(&m_asyncUnzipper, SIGNAL(overwriteRequest(QString)), this, SLOT(unzipperOverwriteRequest(QString)), Qt::QueuedConnection);

    connect(&m_asyncDeleter, SIGNAL(finished()), this, SLOT(filesDeleted()), Qt::QueuedConnection);
}

void ESModElement::Download()
{
    m_failedDownloadsCount = 0;

    if (state != Available && state != InstalledHasUpdate && state != Failed)
        return;

    if (files.empty() || m_uri.isEmpty())
        return;

    blockGui(ByDownload);

    subDownload();
}

void ESModElement::subDownload()
{
    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    connect(this, SIGNAL(abortProcessing()), &m_asyncDownloader, SLOT(abort()));

    if (m_asyncDownloader.downloadFileList(m_uri, files, m_path))
        changeState(Downloading, progress == 100 ? -1 : progress);
}

void ESModElement::Abort()
{
    blockGui(ByAbort);
    emit abortProcessing();
}

void ESModElement::Update()
{
    blockGui(ByUpdate);
    m_asyncDeleter.wait();
    m_asyncDeleter.deleteFiles(m_localFiles);
}

void ESModElement::Delete()
{
    blockGui(ByDelete);

    QStringList allFiles = m_localFiles;
    foreach (const QString &srvFile, files)
        allFiles << QDir(m_path).filePath(srvFile);

    m_asyncDeleter.wait();
    m_asyncDeleter.deleteFiles(allFiles);
}

void ESModElement::SendLike(LikeType l)
{
    mylikemark = LikeMarkNotFound;
    likemarkscount = -1;
    dislikemarkscount = -1;
    emit stateChanged();

    QString setLikeReq = QString("%1?operation=mark&id=%2&mac=%3&udid=%4&platform=%5&mark=%6")\
            .arg(LIKES_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getMacAddress())\
            .arg(AsyncDownloader::getDeviceUDID())\
            .arg(MY_PLATFORM)\
            .arg(l == DislikeMark ? 0 : 1);
    QNetworkReply *setLikeRep = AsyncDownloader::get(setLikeReq);
    connect(setLikeRep, SIGNAL(finished()), this, SLOT(myLikePosted()));
}

void ESModElement::RequestHeaders()
{
    if (files.empty() || m_uri.isEmpty())
    {
        changeState(Installed);
        return;
    }

    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    m_asyncDownloader.downloadFileList(m_uri, files, QString(), true);
}

QString ESModElement::errorString()
{
    if (m_asyncDownloader.failed() || m_asyncDownloader.aborted())
        return m_asyncDownloader.errorString();

    if (m_asyncUnzipper.failed() || m_asyncUnzipper.aborted())
        return m_asyncUnzipper.errorString();

    return tr("Unknown error");
}

void ESModElement::headersReceived()
{
    if (state != Unknown)
        return;

    if (m_asyncDownloader.aborted() || m_asyncDownloader.failed())
    {
        if (m_localFiles.empty())
            changeState(Failed);
        else
            changeState(Installed);

        return;
    }

    m_asyncDownloader.getHeadersData(size, timestamp);
    int resumedProgress = m_asyncDownloader.resumedProgress(files, m_path);
    if (resumedProgress == 0)
        resumedProgress = -1;

    if (m_localFiles.empty())
    {
        changeState(Available, resumedProgress);
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

    m_localFiles << m_asyncUnzipper.unpackedFiles();

    if (state != Unpacking || m_asyncUnzipper.aborted() || m_asyncUnzipper.failed())
    {
        if (m_asyncUnzipper.failed())
            blockGui(ByUnknown); // Without press any button

        m_asyncDeleter.wait();
        m_asyncDeleter.deleteFiles(m_localFiles);
        return;
    }

    QStringList::iterator it = m_localFiles.begin();
    while (it != m_localFiles.end())
        if (it->endsWith(".zip", Qt::CaseInsensitive))
            it = m_localFiles.erase(it);
        else
            ++it;

    m_localTimestamp = timestamp;
    m_localSize = size;

    emit saveMe();
    sendStatistics();
    changeState(InstalledAvailable);
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
        if (m_asyncDownloader.failed())
        {
            // blockGui(ByUnknown); // Without press any button

            m_localFiles.clear();
            m_localSize = 0;
            m_localTimestamp = 0;
            emit saveMe();

            if (m_failedDownloadsCount < 3 && !m_asyncDownloader.failedByDisk())
            {
                ++m_failedDownloadsCount;
                QTimer::singleShot(3000, this, SLOT(subDownload()));
            }
            else
            {
                changeState(Failed, progress);
            }
        }

        if (m_asyncDownloader.aborted())
        {
            m_asyncDeleter.wait();
            m_asyncDeleter.deleteFiles(m_localFiles);
        }

        return;
    }

    QStringList zipList;
    foreach (const QString &zipFile, files)
        if (zipFile.endsWith(".zip", Qt::CaseInsensitive))
            zipList << QDir(m_path).filePath(zipFile);

    connect(this, SIGNAL(abortProcessing()), &m_asyncUnzipper, SLOT(abort()), Qt::QueuedConnection);

    if (m_asyncUnzipper.unzipList(zipList, m_path))
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

    m_failedDownloadsCount = 0;
}

void ESModElement::filesDeleted()
{
    m_localFiles.clear();
    m_localSize = 0;
    m_localTimestamp = 0;

    emit saveMe();

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
        sendStatistics(false);
        changeState(Available);
        break;

    case InstalledHasUpdate :
        if (guiblocked == ByUpdate) // By update button
        {
            state = Available;
            Download();
        }
        if (guiblocked == ByDelete) // By delete button
        {
            sendStatistics(false);
            changeState(Available);
        }
        break;

    case Installed :
        sendStatistics(false);
        state = Unknown;
        deleteLater();
        emit removeMe();
        break;

    case Unknown:
    case Available:
    case Failed:
        changeState(Available);
        break;
    }
}

void ESModElement::unzipperOverwriteRequest(QString fname)
{
    QMessageBox::StandardButton b = QMessageBox::warning(NULL, tr("Risk of overwriting"), \
                                                         tr("File %1 already exists, do you want to overwrite it?").arg(fname), \
                                                         QMessageBox::Cancel | QMessageBox::Yes | QMessageBox::YesToAll, \
                                                         QMessageBox::Cancel);

    m_asyncUnzipper.setOverwriteFlags((b == QMessageBox::YesToAll || b == QMessageBox::Yes), b == QMessageBox::YesToAll);
}

static bool checkLikeResponse(QNetworkReply *rep, QJsonObject &obj)
{
    rep->deleteLater();

    if (rep->error() != QNetworkReply::NoError)
        return false;

    QByteArray data = rep->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull())
        return false;

    if (!doc.isObject())
        return false;

    obj = doc.object();

    if (obj["result"].toString() != "ok")
        return false;

    return true;
}

void ESModElement::allLikesReceived()
{
    QJsonObject obj;
    if (!checkLikeResponse(dynamic_cast<QNetworkReply *>(sender()), obj))
        return;

    likemarkscount = obj["up"].toInt();
    dislikemarkscount = obj["down"].toInt();

    emit stateChanged();
}

void ESModElement::myLikeReceived()
{
    QJsonObject obj;
    if (!checkLikeResponse(dynamic_cast<QNetworkReply *>(sender()), obj))
        return;

    int imrk = obj["mark"].toInt();
    if (imrk < 0)
        mylikemark = LikeMarkNotFound;
    else if (imrk == 0)
        mylikemark = DislikeMark;
    else
        mylikemark = LikeMark;

    emit stateChanged();
}

void ESModElement::myLikePosted()
{
    QJsonObject obj;
    if (!checkLikeResponse(dynamic_cast<QNetworkReply *>(sender()), obj))
        return;

    sendLikesRequests();
}

QJsonObject ESModElement::SerializeToDB()
{
    QJsonArray files_arr;
    foreach (const QString &fname, m_localFiles)
        files_arr << fname;

    QJsonArray langs_arr;
    foreach (const QString &lang, langs)
        langs_arr << lang;

    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["langs"] = langs_arr;
    obj["status"] = status;
    obj["infouri"] = infouri;
    obj["files"] = files_arr;
    obj["size"] = m_localSize;
    obj["timestamp"] = m_localTimestamp;

    return obj;
}

void ESModElement::DeserializeFromDB(const QJsonObject &obj)
{
    id = obj["id"].toInt(-1);
    title = obj["title"].toString();
    status = obj["status"].toString();
    infouri = obj["infouri"].toString();
    m_localSize = obj["size"].toDouble();
    m_localTimestamp = obj["timestamp"].toDouble();

    QJsonArray files_arr = obj["files"].toArray();
    m_localFiles.clear();
    for (int i = 0; i < files_arr.size(); ++i)
        m_localFiles << files_arr[i].toString();

    QJsonArray langs_arr = obj["langs"].toArray();
    langs.clear();
    for (int i = 0; i < langs_arr.size(); ++i)
        langs << langs_arr[i].toString();
}

bool ESModElement::DeserializeFromNetwork(const QJsonObject &obj)
{
    bool myPlatforFound = false;
    QJsonArray platf_arr = obj["platforms"].toArray();
    for (int i = 0; i < platf_arr.size(); ++i)
        if (QString::compare(platf_arr[i].toString().trimmed(), MY_PLATFORM, Qt::CaseInsensitive) == 0)
        {
            myPlatforFound = true;
            break;
        }

    if (!myPlatforFound)
        return false;

    id = obj["idmod"].toInt(-1);
    title = obj["title"].toString().trimmed();
    status = obj["status"].toString().trimmed();
    langs = obj["lang"].toString().trimmed().split(QRegExp("[,\\s]+"), QString::SkipEmptyParts);
    infouri = obj[QString("infouri_") + MY_PLATFORM].toString().trimmed();

    QJsonArray files_arr = obj[QString("files_") + MY_PLATFORM].toArray();
    for (int i = 0; i < files_arr.size(); ++i)
        files << files_arr[i].toString().trimmed();

    return true;
}

bool ESModElement::DeserializeFromAllLikesList(const QJsonObject &obj)
{
    if (obj["id"].toInt() != id)
        return false;

    int imrk = obj["mark"].toInt();
    if (imrk < 0)
        mylikemark = LikeMarkNotFound;
    else if (imrk == 0)
        mylikemark = DislikeMark;
    else
        mylikemark = LikeMark;

    likemarkscount = obj["up"].toInt();
    dislikemarkscount = obj["down"].toInt();

    return true;
}

void ESModElement::DeserializeFromAllStatisticsList(const QJsonObject &obj)
{
    if (obj["id"].toInt() != id)
        return;

    insttotal = obj["instcount_all"].toInt();
    instactive = obj["instactive_all"].toInt();
    insttotalmonth = obj["instcount_month"].toInt();
    instactivemonth = obj["instactive_month"].toInt();
    insttotalweek = obj["instcount_week"].toInt();
    instactiveweek = obj["instactive_week"].toInt();
    lifetimeavg = obj["lifetime_avg"].toInt();
    lifetimemax = obj["lifetime_max"].toInt();
}

void ESModElement::SetInstallPath(QString p)
{
    m_path = p;
}

void ESModElement::TryToPickupFrom(QList<ESModElement *> &list)
{
    QList<ESModElement *>::iterator it = list.begin();
    while (it != list.end())
    {
        if (idEquals(*it))
        {
            if ((*it)->id != -1)
                id = (*it)->id;

            m_localFiles = (*it)->m_localFiles;
            m_localSize = (*it)->m_localSize;
            m_localTimestamp = (*it)->m_localTimestamp;
            delete (*it);
            it = list.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ESModElement::blockGui(GuiBlockReason b)
{
    guiblocked = b;
    emit stateChanged();
}

void ESModElement::changeState(State s, int resumedProgress)
{
    if (resumedProgress >= 0)
    {
        progress = resumedProgress;
    }
    else
    {
        if (s == Downloading || s == Unpacking)
            progress = 0;
        else
            progress = 100;
    }

    guiblocked = NoBlock;
    state = s;
    emit stateChanged();
}

void ESModElement::sendLikesRequests()
{
    QString myLikeReq = QString("%1?operation=mymark&id=%2&mac=%3&udid=%4")\
            .arg(LIKES_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getMacAddress())\
            .arg(AsyncDownloader::getDeviceUDID());
    QNetworkReply *myLikeRep = AsyncDownloader::get(myLikeReq);
    connect(myLikeRep, SIGNAL(finished()), this, SLOT(myLikeReceived()));

    QString allLikeReq = QString("%1?operation=query&id=%2").arg(LIKES_CGI_URL).arg(id);
    QNetworkReply *allLikeRep = AsyncDownloader::get(allLikeReq);
    connect(allLikeRep, SIGNAL(finished()), this, SLOT(allLikesReceived()));
}

void ESModElement::sendStatistics(bool inst)
{
    QString statReq = QString("%1?operation=statistics&id=%2&mac=%3&udid=%4&platform=%5&state=%6")\
            .arg(STATS_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getMacAddress())\
            .arg(AsyncDownloader::getDeviceUDID())\
            .arg(MY_PLATFORM)\
            .arg(inst ? "installed" : "deleted");

    StatisticsManager::getInstance()->addRequest(statReq);
}

bool ESModElement::idEquals(ESModElement *el)
{
    if (id != -1 && id == el->id)
        return true;

    QString myTitle = this->title;
    myTitle = myTitle.remove(QRegExp("\\(\\b(?:Ru|Eng|Spa|,)\\b\\)")).remove(QRegExp("\\[.*\\]")).remove(QRegExp("\\{.*\\}")).simplified();

    QString title2 = el->title;
    title2 = title2.remove(QRegExp("\\(\\b(?:Ru|Eng|Spa|,)\\b\\)")).remove(QRegExp("\\[.*\\]")).remove(QRegExp("\\{.*\\}")).simplified();

    return (myTitle == title2);
}
