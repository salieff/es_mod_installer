#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QNetworkProxy>
#include <QSettings>

#include "esmodmodel.h"

#define ES_MOD_INDEX_URL "http://191.ru/es/project1.json"

ESModModel::ESModModel(QObject *parent)
    : QAbstractListModel(parent),
      m_JsonWriter(this),
      m_lastSortMode(AsServer)
{
#if !defined(ANDROID) && !defined(Q_OS_IOS)
    // AsyncDownloader::NetworkManager.setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 3128));
#endif

#ifdef Q_OS_IOS
    m_iosEverlastingSummerFolder = ESFolderForIOS(QStringList() \
                                                  << "/var/mobile/Containers/Bundle/Application" \
                                                  << "/var/mobile/Applications" \
                                                  << "/Applications");

    if (m_iosEverlastingSummerFolder.isEmpty())
        QMessageBox::critical(NULL, tr("Error"), tr("Can't find Everlasting Summer installation folder"));
    else
        QMessageBox::information(NULL, tr("Everlasting Summer"), tr("Installed in ") + m_iosEverlastingSummerFolder);
#endif

    m_JsonWriter.start();

    QNetworkReply *rep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(ES_MOD_INDEX_URL)));
    connect(rep, SIGNAL(finished()), this, SLOT(ESModIndexDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(ESModIndexError(QNetworkReply::NetworkError)));
}

ESModModel::~ESModModel()
{
    SaveLocalModsDB();
    m_JsonWriter.close();
    m_JsonWriter.wait();
}

void ESModModel::addModElement(ESModElement *element)
{
    if (m_initialElements.empty())
    {
        element->mylikemark = ESModElement::LikeMarkNotFound;
        element->likemarkscount = 0;
        element->dislikemarkscount = 0;
    }
    else
    {
        element->mylikemark = (ESModElement::LikeType)(qrand() % 3);
        element->likemarkscount = qrand() % 100;
        element->dislikemarkscount = qrand() % 100;
    }

    m_initialElements << element;
    connect(element, SIGNAL(stateChanged()), this, SLOT(elementChanged()));
    connect(element, SIGNAL(saveMe()), this, SLOT(SaveLocalModsDB()));
    connect(element, SIGNAL(removeMe()), this, SLOT(elementNeedRemove()));
}

int ESModModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return m_elements.count();
}

QVariant ESModModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_elements.count())
        return QVariant();

    const ESModElement *element = m_elements[index.row()];

    switch (role)
    {
    case TitleRole:
        return element->title;
        break;

    case StatusRole:
        return element->status;
        break;

    case LangsRole:
        return element->langs.join(",");
        break;

    case UriRole:
        return element->uri;
        break;

    case InfoUriRole:
        return element->infouri;
        break;

    case PathRole:
        return element->path;
        break;

    case FilesRole:
        return element->files.join("\n");
        break;

    case StateRole:
        return element->state;
        break;

    case ProgressRole:
        return element->progress;
        break;

    case SizeRole:
        if (element->size == 0)
            return element->m_localSize;

        return element->size;
        break;

    case TimestampRole:
    {
        QDateTime dt;

        if (element->timestamp == 0)
            dt.setTime_t(element->m_localTimestamp);
        else
            dt.setTime_t(element->timestamp);

        return dt.toString("yyyy.MM.dd");
    }
        break;

    case GuiBlockedRole:
        return element->guiblocked;
        break;

    case MyLikeMarkRole :
        return element->mylikemark;
        break;

    case LikeMarksCountRole :
        return element->likemarkscount;
        break;

    case DislikeMarksCountRole :
        return element->dislikemarkscount;
        break;

    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> ESModModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[StatusRole] = "status";
    roles[LangsRole] = "langs";
    roles[UriRole] = "uri";
    roles[InfoUriRole] = "infouri";
    roles[PathRole] = "path";
    roles[FilesRole] = "files";
    roles[StateRole] = "modstate";
    roles[ProgressRole] = "progress";
    roles[SizeRole] = "modsize";
    roles[TimestampRole] = "timestamp";
    roles[GuiBlockedRole] = "guiblocked";
    roles[MyLikeMarkRole] = "mylikemark";
    roles[LikeMarksCountRole] = "likemarkscount";
    roles[DislikeMarksCountRole] = "dislikemarkscount";

    return roles;
}

void ESModModel::ESModIndexDownloaded()
{
    QList<ESModElement *> local_elements;
    LoadLocalModsDB(local_elements);

    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    if (rep->error() == QNetworkReply::NoError)
    {
        QByteArray data = rep->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);

        if (doc.isNull())
        {
            QMessageBox::critical(NULL, tr("Index isn't valid JSON"), err.errorString());
        }
        else if (!doc.isObject())
        {
            QMessageBox::critical(NULL, tr("Bad index"), tr("Index doesn't contain any JSON objects"));
        }
        else
        {
            QJsonObject obj = doc.object();

            QString appTitle = obj["appTitle"].toString();
            if (!appTitle.isEmpty())
                emit appTitleReceived(appTitle);

            QString appHelp = obj["appReadMe"].toString();
            if (!appHelp.isEmpty())
                emit appHelpReceived(appHelp);

            QJsonArray arr = obj["packs"].toArray();
            for (int i = 0; i < arr.size(); ++i)
            {
                ESModElement *el = new ESModElement(this);
                el->DeserializeFromNetwork(arr[i].toObject());
                el->TryToPickupFrom(local_elements);
                addModElement(el);
            }
        }
    }

    foreach (ESModElement *el, local_elements)
        addModElement(el);

    sortList(m_lastSortMode);

    foreach (ESModElement *el, m_elements)
        el->RequestHeaders();

    emit esIndexReceived();
    rep->deleteLater();
}

void ESModModel::ESModIndexError(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);

    sortList(m_lastSortMode);
    emit esIndexReceived();

    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    QMessageBox::critical(NULL, "Index download error", rep->errorString());
}

void ESModModel::Download(int ind)
{
    m_elements[ind]->Download();
}

void ESModModel::Abort(int ind)
{
    m_elements[ind]->Abort();
}

void ESModModel::Retry(int ind)
{
    m_elements[ind]->Download();
}

void ESModModel::Update(int ind)
{
    m_elements[ind]->Update();
}

void ESModModel::Delete(int ind)
{
    m_elements[ind]->Delete();
}

void ESModModel::SendLike(int ind, int l)
{
    m_elements[ind]->SendLike((ESModElement::LikeType)l);
}

void ESModModel::ShowError(int ind)
{
    if (m_elements[ind]->state != ESModElement::Failed)
        return;

    QMessageBox::critical(NULL, tr("Error"), m_elements[ind]->errorString());
}

void ESModModel::elementChanged()
{
    ESModElement *el = dynamic_cast<ESModElement *>(sender());

    if (el->m_modelIndex >= 0)
        emit dataChanged(index(el->m_modelIndex, 0), index(el->m_modelIndex, 0));
}

void ESModModel::elementNeedRemove()
{
    ESModElement *el = dynamic_cast<ESModElement *>(sender());

    beginRemoveRows(QModelIndex(), el->m_modelIndex, el->m_modelIndex);
    QList<ESModElement *>::iterator it = m_initialElements.begin();
    while(it != m_initialElements.end())
        if (el == (*it))
        {
            it = m_initialElements.erase(it);
            break;
        }
        else
        {
            ++it;
        }

    m_elements.removeAt(el->m_modelIndex);
    ReindexElements();
    endRemoveRows();
}

bool ESModModel::LoadLocalModsDB(QList<ESModElement *> &l)
{
#ifndef ANDROID
    QFile f(QString(ES_MOD_DB_PATH).replace(QRegExp("^/sdcard/Android/data"), QDir::homePath() + "/tmp"));
#else
    QFile f(ES_MOD_DB_PATH);
#endif

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray data = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (doc.isNull())
    {
        QMessageBox::critical(NULL, tr("DB isn't valid JSON"), err.errorString());
        return false;
    }

    if (!doc.isObject())
    {
        QMessageBox::critical(NULL, tr("Bad DB"), tr("DB doesn't contain any JSON objects"));
        return false;
    }

    QJsonObject obj = doc.object();

    // FIXME: Can't sort by size or date until headers weren't received
    // m_lastSortMode = (ESModModel::SortMode)obj["sortmode"].toInt();

    QJsonArray arr = obj["packs"].toArray();
    for (int i = 0; i < arr.size(); ++i)
    {
        ESModElement *el = new ESModElement(this);
        el->DeserializeFromDB(arr[i].toObject());
        l << el;
    }

    return true;
}

void ESModModel::SaveLocalModsDB()
{
    QJsonArray arr;
    for (int i = 0; i < m_initialElements.size(); ++i)
        if (!m_initialElements[i]->m_localFiles.empty())
            arr.push_back(m_initialElements[i]->SerializeToDB());

    QJsonObject *obj = new QJsonObject;
    obj->insert("sortmode", (int)m_lastSortMode);
    obj->insert("packs", arr);
    m_JsonWriter.write(obj);
}

static const char *statusNamesArr[] = { \
    "окончен", \
    "в разработке", \
    "заморожен", \
    "демо", \
    "надстройка" \
};

static bool lessThanAsServer(ESModElement *a, ESModElement *b)
{
    int i1 = -1;
    int i2 = -1;

    for (size_t i = 0; i < sizeof(statusNamesArr) / sizeof(statusNamesArr[0]); ++i)
    {
        if (a->status.compare(statusNamesArr[i], Qt::CaseInsensitive) == 0)
            i1 = i;

        if (b->status.compare(statusNamesArr[i], Qt::CaseInsensitive) == 0)
            i2 = i;
    }
    if (i1 == i2)
        return a->title < b->title;

    return i1 < i2;
}

static bool lessThanByName0(ESModElement *a, ESModElement *b)
{
    return a->title < b->title;
}

static bool lessThanByName1(ESModElement *a, ESModElement *b)
{
    return a->title >= b->title;
}

static bool lessThanBySize0(ESModElement *a, ESModElement *b)
{
    double sz1 = a->size;
    if (sz1 == 0)
        sz1 = a->m_localSize;

    double sz2 = b->size;
    if (sz2 == 0)
        sz2 = b->m_localSize;

    return sz1 < sz2;
}

static bool lessThanBySize1(ESModElement *a, ESModElement *b)
{
    double sz1 = a->size;
    if (sz1 == 0)
        sz1 = a->m_localSize;

    double sz2 = b->size;
    if (sz2 == 0)
        sz2 = b->m_localSize;

    return sz1 >= sz2;
}

static bool lessThanByDate0(ESModElement *a, ESModElement *b)
{
    double tm1 = a->timestamp;
    if (tm1 == 0)
        tm1 = a->m_localTimestamp;

    double tm2 = b->timestamp;
    if (tm2 == 0)
        tm2 = b->m_localTimestamp;

    return tm1 < tm2;
}

static bool lessThanByDate1(ESModElement *a, ESModElement *b)
{
    double tm1 = a->timestamp;
    if (tm1 == 0)
        tm1 = a->m_localTimestamp;

    double tm2 = b->timestamp;
    if (tm2 == 0)
        tm2 = b->m_localTimestamp;

    return tm1 >= tm2;
}

typedef bool (*lessThanSortFunc)(ESModElement *a, ESModElement *b);
static const lessThanSortFunc lessThanArray[] = { \
    lessThanAsServer, \
    lessThanByName0, \
    lessThanByName1, \
    lessThanBySize0, \
    lessThanBySize1, \
    lessThanByDate0, \
    lessThanByDate1 \
};

void ESModModel::sortList(SortMode m)
{
    m_lastSortMode = m;

    beginResetModel();
    m_elements = m_initialElements;
    if (lessThanArray[m])
        qSort(m_elements.begin(), m_elements.end(), lessThanArray[m]);
    ReindexElements();
    endResetModel();

    emit listSorted(m_lastSortMode);
}

static bool lessThanKeyword(ESModElement *a, ESModElement *b)
{
    if (a->m_keywordFilterCounter.size() != b->m_keywordFilterCounter.size())
        return a->m_keywordFilterCounter.size() > b->m_keywordFilterCounter.size();

    for (size_t i = 0; i < a->m_keywordFilterCounter.size(); ++i)
    {
        if (a->m_keywordFilterCounter[i] == b->m_keywordFilterCounter[i])
            continue;

        return a->m_keywordFilterCounter[i] >= b->m_keywordFilterCounter[i];
    }

    return false;
}

void ESModModel::filterByKeywords(QString str)
{
    if (str.isEmpty())
    {
        sortList(m_lastSortMode);
        return;
    }

    QStringList strList = str.trimmed().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QStringList::iterator sit = strList.begin();
    while (sit != strList.end())
    {
        if (sit->length() < 3)
            sit = strList.erase(sit);
        else
            ++sit;
    }

    if (strList.empty())
        return;

    beginResetModel();
    m_elements = m_initialElements;

    QList<ESModElement *>::iterator it = m_elements.begin();
    while (it != m_elements.end())
    {
        (*it)->m_keywordFilterCounter.assign(strList.size(), 0);

        bool found = false;
        for (int i = 0; i < strList.size(); ++i)
            if ((*it)->title.contains(strList[i], Qt::CaseInsensitive))
            {
                found = true;
                (*it)->m_keywordFilterCounter[i]++;
            }

        if (found)
        {
            ++it;
            continue;
        }

        it = m_elements.erase(it);
    }

    qSort(m_elements.begin(), m_elements.end(), lessThanKeyword);
    ReindexElements();
    endResetModel();
}

void ESModModel::ReindexElements()
{
    for (int i = 0; i < m_elements.size(); ++i)
        m_initialElements[i]->m_modelIndex = -1;

    for (int i = 0; i < m_elements.size(); ++i)
        m_elements[i]->m_modelIndex = i;
}

#ifdef Q_OS_IOS
QString ESModModel::ESFolderForIOS(QStringList &dirs)
{
    foreach (QString dir, dirs) // Top application directories
    {
        QFileInfoList uuidlist = QDir(dir).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach (QFileInfo fiuuid, uuidlist) // Application UUIDs directories
        {
            QFileInfoList applist = fiuuid.absoluteDir().entryInfoList(QStringList("*.app"), QDir::Dirs | QDir::NoDotAndDotDot);
            foreach (QFileInfo fiapp, applist) // *.app directories
            {
                QFileInfo iplist(fiapp.absoluteDir(), "Info.plist");
                if (!iplist.isFile())
                    continue;

                QString bunid = QSettings(iplist.absoluteFilePath(), QSettings::NativeFormat).value("CFBundleIdentifier").toString();
                if (bunid != "com.mifki.everlastingsummer")
                    continue;

                return fiapp.absoluteFilePath();
            }
        }
    }

    return QString();
}
#endif
