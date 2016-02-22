#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>

#include "esmodelement.h"

#define LIKES_CGI_URL "http://es.191.ru/cgi-bin/ratingsystem/rating_web.py"
#define STATS_CGI_URL LIKES_CGI_URL

#if defined(Q_OS_IOS)
    #define MY_PLATFORM "ios"
#elif defined(ANDROID)
    #define MY_PLATFORM "android"
#else
    #define MY_PLATFORM "ios"
//    #define MY_PLATFORM "android"
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
      guiblocked(ByUnknown),
      m_localSize(0),
      m_localTimestamp(0),
      m_modelIndex(-1),
      m_uri(au),
      m_path(ap)
{
    connect(&m_asyncDownloader, SIGNAL(progress(int)), this, SLOT(downloadProgress(int)));
    connect(&m_asyncDownloader, SIGNAL(finished()), this, SLOT(filesDownloaded()));
    connect(&m_asyncDownloader, SIGNAL(headersReady()), this, SLOT(headersReceived()));

    connect(&m_asyncUnzipper, SIGNAL(finished()), this, SLOT(zipListUnpacked()));
    connect(&m_asyncUnzipper, SIGNAL(progress(int)), this, SLOT(unpackProgress(int)));
    connect(&m_asyncUnzipper, SIGNAL(overwriteRequest(QString)), this, SLOT(unzipperOverwriteRequest(QString)));

    connect(&m_asyncDeleter, SIGNAL(finished()), this, SLOT(filesDeleted()));
}

void ESModElement::Download()
{
    if (state != Available && state != InstalledHasUpdate && state != Failed)
        return;

    if (files.empty() || m_uri.isEmpty())
        return;

    blockGui(ByDownload);

    // Make shure previous async operations already done
    m_asyncDownloader.wait();
    m_asyncUnzipper.wait();

    connect(this, SIGNAL(abortProcessing()), &m_asyncDownloader, SLOT(abort()));

    if (!m_asyncDownloader.downloadFileList(m_uri, files, m_path))
        changeState(Failed);
    else
        changeState(Downloading);
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
    m_asyncDeleter.wait();
    m_asyncDeleter.deleteFiles(m_localFiles);
}

void ESModElement::SendLike(LikeType l)
{
    QString setLikeReq = QString("%1?operation=mark&id=%2&mac=%3&mark=%4")\
            .arg(LIKES_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getMacAddress())\
            .arg(l == DislikeMark ? 0 : 1);
    QNetworkReply *setLikeRep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(setLikeReq)));
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

    sendLikesRequests();
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
            blockGui(ByUnknown); // Without press any button

        m_asyncDeleter.wait();
        m_asyncDeleter.deleteFiles(m_localFiles);
        return;
    }

    QStringList zipList;
    foreach (const QString &zipFile, files)
        if (zipFile.endsWith(".zip", Qt::CaseInsensitive))
            zipList << QDir(m_path).filePath(zipFile);

    connect(this, SIGNAL(abortProcessing()), &m_asyncUnzipper, SLOT(abort()));

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
        // These states don't call Delete()
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
        if (platf_arr[i].toString().trimmed() == MY_PLATFORM)
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

void ESModElement::changeState(State s)
{
    if (s == Downloading || s == Unpacking)
        progress = 0;
    else
        progress = 100;

    guiblocked = NoBlock;
    state = s;
    emit stateChanged();
}

void ESModElement::sendLikesRequests()
{
    QString myLikeReq = QString("%1?operation=mymark&id=%2&mac=%3")\
            .arg(LIKES_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getMacAddress());
    QNetworkReply *myLikeRep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(myLikeReq)));
    connect(myLikeRep, SIGNAL(finished()), this, SLOT(myLikeReceived()));

    QString allLikeReq = QString("%1?operation=query&id=%2").arg(LIKES_CGI_URL).arg(id);
    QNetworkReply *allLikeRep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(allLikeReq)));
    connect(allLikeRep, SIGNAL(finished()), this, SLOT(allLikesReceived()));
}

void ESModElement::sendStatistics(bool inst)
{
    QString statReq = QString("%1?operation=statistics&id=%2&mac=%3&state=%4")\
            .arg(STATS_CGI_URL)\
            .arg(id)\
            .arg(AsyncDownloader::getMacAddress())\
            .arg(inst ? "installed" : "deleted");
    QNetworkReply *myStatRep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(statReq)));
    connect(myStatRep, SIGNAL(finished()), myStatRep, SLOT(deleteLater()));
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
