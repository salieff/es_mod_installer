#include <QDir>
#include <QJsonArray>
#include <QMessageBox>

#include "esmodelement.h"

ESModElement::ESModElement(QObject *parent, State s, int p)
    : QObject(parent),
      id(-1),
      title(QStringLiteral("Test sample mod name")),
      state(s),
      progress(p),
      size(0),
      timestamp(0),
      mylikemark(LikeMarkNotFound),
      likemarkscount(0),
      dislikemarkscount(0),
      guiblocked(ByUnknown),
      m_localSize(0),
      m_localTimestamp(0),
      m_modelIndex(0)
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

    if (files.empty() || uri.isEmpty())
        return;

    blockGui(ByDownload);

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
            changeState(Available);
        }
        break;

    case Installed :
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

void ESModElement::DeserializeFromNetwork(const QJsonObject &obj, QString overridePath)
{
    id = obj["idmod"].toInt(-1);
    title = obj["title"].toString().trimmed();
    status = obj["status"].toString().trimmed();
    langs = obj["lang"].toString().trimmed().split(QRegExp("[,\\s]+"), QString::SkipEmptyParts);
    uri = obj["uri"].toString().trimmed();
    infouri = obj["infouri"].toString().trimmed();

    if (overridePath.isEmpty())
        path = obj["path"].toString().trimmed();
    else
        path = overridePath;

    QJsonArray files_arr = obj["files"].toArray();
    for (int j = 0; j < files_arr.size(); ++j)
        files << files_arr[j].toString().trimmed();
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
