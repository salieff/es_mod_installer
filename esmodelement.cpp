﻿#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QTimer>

#include "esmodelement.h"
#include "statisticsmanager.h"
#include "safadapter.h"

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
      state(st),
      progress(pr),
      m_uri(au),
      m_path(ap)
{
    connect(&m_asyncDownloader, SIGNAL(progress(int)), this, SLOT(downloadProgress(int)));
    connect(&m_asyncDownloader, SIGNAL(finished()), this, SLOT(filesDownloaded()));
    connect(&m_asyncDownloader, SIGNAL(headersReady()), this, SLOT(headersReceived()));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()), Qt::QueuedConnection);
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)), Qt::QueuedConnection);
    connect(&m_asyncUnzipper, SIGNAL(overwriteRequest(QString)), this, SLOT(unzipperOverwriteRequest(QString)), Qt::QueuedConnection);

    connect(&m_asyncDeleter, SIGNAL(finished()), this, SLOT(filesDeleted()), Qt::QueuedConnection);

    if (m_path != ANDROID_ES_MODS_FOLDER_DATA)
    {
        m_asyncUnzipper.setStopFolder(m_path);
        m_asyncDeleter.setStopFolder(m_path);
    }
}

void ESModElement::Download(void)
{
    m_failedDownloadsCount = 0;

    if (state != Available && state != InstalledHasUpdate && state != Failed)
        return;

    if (files.empty() || m_uri.isEmpty())
        return;

    blockGui(ByDownload);

    subDownload();
}

void ESModElement::subDownload(void)
{
    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    connect(this, SIGNAL(abortProcessing()), &m_asyncDownloader, SLOT(abort()));

    if (m_asyncDownloader.downloadFileList(m_uri, files, m_path))
        changeState(Downloading, progress == 100 ? -1 : progress);
}

void ESModElement::Abort(void)
{
    blockGui(ByAbort);
    emit abortProcessing();
}

void ESModElement::Update(void)
{
    blockGui(ByUpdate);
    m_asyncDeleter.wait();

    m_asyncDeleter.deleteFiles(LocalFiles());
}

void ESModElement::Delete(void)
{
    blockGui(ByDelete);

    QStringList allFiles = LocalFiles();

    for(const QString &srvFile : files)
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

    QString setLikeReq = QString("%1?operation=mark&id=%2&udid=%3&platform=%4&mark=%5")\
            .arg(LIKES_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getDeviceUDID())\
            .arg(MY_PLATFORM)\
            .arg(l == DislikeMark ? 0 : 1);
    QNetworkReply *setLikeRep = AsyncDownloader::get(setLikeReq);
    connect(setLikeRep, SIGNAL(finished()), this, SLOT(myLikePosted()));
}

void ESModElement::ToggleFavorite(void)
{
    favorite = !favorite;
    emit saveMe();
    emit stateChanged();
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
        if (LocalFiles().isEmpty())
            changeState(Failed);
        else
            changeState(Installed);

        return;
    }

    m_asyncDownloader.getHeadersData(size, timestamp);

    int resumedProgress = m_asyncDownloader.resumedProgress(files, m_path);
    if (resumedProgress == 0)
        resumedProgress = -1;

    if (LocalFiles().isEmpty())
    {
        changeState(Available, resumedProgress);
    }
    else
    {
        if (timestamp > LocalTimeStamp())
            changeState(InstalledHasUpdate);
        else
            changeState(InstalledAvailable);
    }
}

void ESModElement::zipListUnpacked()
{
    disconnect(this, SIGNAL(abortProcessing()), &m_asyncUnzipper, SLOT(abort()));

    AddToLocalFiles(m_asyncUnzipper.unpackedFiles());

    if (state != Unpacking || m_asyncUnzipper.aborted() || m_asyncUnzipper.failed())
    {
        if (m_asyncUnzipper.failed())
            blockGui(ByUnknown); // Without press any button

        m_asyncDeleter.wait();
        m_asyncDeleter.deleteFiles(LocalFiles());

        return;
    }

    EraseFromLocalFiles(".zip");

    SetLocalTimeStamp(timestamp);
    SetLocalSize(size);

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
    AddToLocalFiles(m_asyncDownloader.downloadedFiles(), REPLACE_LOCAL_FILES);

    if (state != Downloading || m_asyncDownloader.aborted() || m_asyncDownloader.failed())
    {
        if (m_asyncDownloader.failed())
        {
            // blockGui(ByUnknown); // Without press any button

            ClearLocalFiles();
            SetLocalSize(0);
            SetLocalTimeStamp(0);
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
            m_asyncDeleter.deleteFiles(LocalFiles());
        }

        return;
    }

    QStringList zipList;
    for (const QString &zipFile : files)
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
    ClearLocalFiles();
    SetLocalSize(0);
    SetLocalTimeStamp(0);

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
    QJsonObject files_obj;
    for (const auto &pair : m_localFilesMap)
        files_obj[pair.first] = QJsonArray::fromStringList(pair.second);

    QJsonObject sizes_obj;
    for (const auto &pair : m_localSizesMap)
        sizes_obj[pair.first] = pair.second;

    QJsonObject tmstamps_obj;
    for (const auto &pair : m_localTimestampsMap)
        tmstamps_obj[pair.first] = pair.second;

    QJsonArray langs_arr;
    foreach (const QString &lang, langs)
        langs_arr << lang;

    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["langs"] = langs_arr;
    obj["status"] = status;
    obj["infouri"] = infouri;
    obj["files"] = files_obj;
    obj["size"] = sizes_obj;
    obj["timestamp"] = tmstamps_obj;
    obj["favorite"] = favorite;

    return obj;
}

QString ESModElement::removeOldLocalFilePrefixes(QString filePath)
{
    QString localFile(filePath);

    localFile.remove(QRegularExpression("^\\/sdcard\\/Android\\/data"));
    localFile.remove(QRegularExpression("^Android\\/data"));
    localFile.remove(QRegularExpression("^data"));

    localFile.remove(QRegularExpression("^\\/sdcard\\/Android\\/media"));
    localFile.remove(QRegularExpression("^Android\\/media"));
    localFile.remove(QRegularExpression("^media"));

    return localFile;
}

void ESModElement::addLocalFilesForSafRoot(const QString &safRoot, const QJsonValue jvr)
{
    if (!jvr.isArray())
        return;

    for (const auto &file_element : jvr.toArray())
        m_localFilesMap[safRoot] << removeOldLocalFilePrefixes(file_element.toString());
}

void ESModElement::DeserializeFromDB(const QJsonObject &obj)
{
    id = obj["id"].toInt(-1);
    title = obj["title"].toString();
    status = obj["status"].toString();
    infouri = obj["infouri"].toString();
    favorite = obj["favorite"].toBool();

    m_localSizesMap.clear();
    if (obj["size"].isObject())
    {
        auto szObj = obj["size"].toObject();
        for (QJsonObject::iterator it = szObj.begin(); it != szObj.end(); ++it)
            m_localSizesMap[it.key()] = it.value().toDouble();
    }
    else
    {
        m_localSizesMap[SafAdapter::getCurrentAdapterRoot()] = obj["size"].toDouble();
    }

    m_localTimestampsMap.clear();
    if (obj["timestamp"].isObject())
    {
        auto tmObj = obj["timestamp"].toObject();
        for (QJsonObject::iterator it = tmObj.begin(); it != tmObj.end(); ++it)
            m_localTimestampsMap[it.key()] = it.value().toDouble();
    }
    else
    {
        m_localTimestampsMap[SafAdapter::getCurrentAdapterRoot()] = obj["timestamp"].toDouble();
    }

    m_localFilesMap.clear();
    if (obj["files"].isObject())
    {
        auto fileObj = obj["files"].toObject();
        for (QJsonObject::iterator it = fileObj.begin(); it != fileObj.end(); ++it)
            addLocalFilesForSafRoot(it.key(), it.value());
    }
    else // Backward compatibility
    {
        addLocalFilesForSafRoot(SafAdapter::getCurrentAdapterRoot(), obj["files"]);
    }

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
    langs = obj["lang"].toString().trimmed().split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
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
    m_asyncUnzipper.setStopFolder(p);
    m_asyncDeleter.setStopFolder(p);
}

void ESModElement::TryToPickupFrom(QList<ESModElement *> &list)
{
    auto it = std::find_if(list.begin(), list.end(), [this](ESModElement *el){ return idEquals(el); });
    if (it == list.end())
        return;

    if ((*it)->id != -1)
        id = (*it)->id;

    m_localFilesMap = (*it)->m_localFilesMap;
    m_localSizesMap = (*it)->m_localSizesMap;
    m_localTimestampsMap = (*it)->m_localTimestampsMap;
    favorite = (*it)->favorite;
    delete (*it);
    list.erase(it);
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
    QString myLikeReq = QString("%1?operation=mymark&id=%2&udid=%3")\
            .arg(LIKES_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getDeviceUDID());
    QNetworkReply *myLikeRep = AsyncDownloader::get(myLikeReq);
    connect(myLikeRep, SIGNAL(finished()), this, SLOT(myLikeReceived()));

    QString allLikeReq = QString("%1?operation=query&id=%2").arg(LIKES_CGI_URL).arg(id);
    QNetworkReply *allLikeRep = AsyncDownloader::get(allLikeReq);
    connect(allLikeRep, SIGNAL(finished()), this, SLOT(allLikesReceived()));
}

void ESModElement::sendStatistics(bool inst)
{
    QString statReq = QString("%1?operation=statistics&id=%2&udid=%3&platform=%4&state=%5")\
            .arg(STATS_CGI_URL)\
            .arg(id)\
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
    myTitle = myTitle.remove(QRegularExpression("\\(\\b(?:Ru|Eng|Spa|,)\\b\\)")).remove(QRegularExpression("\\[.*\\]")).remove(QRegularExpression("\\{.*\\}")).simplified();

    QString title2 = el->title;
    title2 = title2.remove(QRegularExpression("\\(\\b(?:Ru|Eng|Spa|,)\\b\\)")).remove(QRegularExpression("\\[.*\\]")).remove(QRegularExpression("\\{.*\\}")).simplified();

    return (myTitle == title2);
}

QStringList ESModElement::LocalFiles(void)
{
    const auto it = m_localFilesMap.find(SafAdapter::getCurrentAdapterRoot());
    if (it == m_localFilesMap.end())
        return QStringList();

    if (it->second.empty())
    {
        m_localFilesMap.erase(it);
        return QStringList();
    }

    return it->second;
}

void ESModElement::AddToLocalFiles(const QStringList &l, bool replace)
{
    if (l.isEmpty())
        return;

    if (replace)
        m_localFilesMap[SafAdapter::getCurrentAdapterRoot()] = l;
    else
        m_localFilesMap[SafAdapter::getCurrentAdapterRoot()] << l;
}

void ESModElement::AddToLocalFiles(const QString &s, bool replace)
{
    if (s.isEmpty())
        return;

    if (replace)
        m_localFilesMap[SafAdapter::getCurrentAdapterRoot()] = QStringList(s);
    else
        m_localFilesMap[SafAdapter::getCurrentAdapterRoot()] << s;
}

void ESModElement::EraseFromLocalFiles(const QString &ext)
{
    const auto it = m_localFilesMap.find(SafAdapter::getCurrentAdapterRoot());
    if (it == m_localFilesMap.end())
        return;

    if (it->second.empty())
        return;

    it->second.erase(std::remove_if(it->second.begin(),
                                    it->second.end(),
                                    [&ext](const QString &s){ return s.endsWith(ext, Qt::CaseInsensitive); }),
                     it->second.end());
}

void ESModElement::ClearLocalFiles(void)
{
    const auto it = m_localFilesMap.find(SafAdapter::getCurrentAdapterRoot());
    if (it == m_localFilesMap.end())
        return;

    it->second.clear();
}

double ESModElement::LocalSize(void)
{
    const auto it = m_localSizesMap.find(SafAdapter::getCurrentAdapterRoot());
    if (it == m_localSizesMap.end())
        return 0;

    if (it->second == 0)
    {
        m_localSizesMap.erase(it);
        return 0;
    }

    return it->second;
}

double ESModElement::LocalTimeStamp(void)
{
    const auto it = m_localTimestampsMap.find(SafAdapter::getCurrentAdapterRoot());
    if (it == m_localTimestampsMap.end())
        return 0;

    if (it->second == 0)
    {
        m_localTimestampsMap.erase(it);
        return 0;
    }

    return it->second;
}

void ESModElement::SetLocalSize(double s)
{
    m_localSizesMap[SafAdapter::getCurrentAdapterRoot()] = s;
}

void ESModElement::SetLocalTimeStamp(double t)
{
    m_localTimestampsMap[SafAdapter::getCurrentAdapterRoot()] = t;
}
