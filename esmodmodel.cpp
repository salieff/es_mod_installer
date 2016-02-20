#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QNetworkProxy>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>

#include "esmodmodel.h"

#define ES_MOD_INDEX_SERVER "http://191.ru/es/"
#define ES_MOD_INDEX_NAME "project2.json"
#define ANDROID_ES_MODS_FOLDER "/sdcard/Android/data/su.sovietgames.everlasting_summer/files/"

ESModModel::ESModModel(QObject *parent)
    : QAbstractListModel(parent),
      m_JsonWriter(this),
      m_lastSortMode(AsServer)
{
#if defined(Q_OS_IOS)
    m_ESModsFolder = ESFolderForIOS(QStringList() \
                                    << "/User/Containers/Bundle/Application" \
                                    << "/User/Applications" \
                                    << "/var/mobile/Containers/Bundle/Application" \
                                    << "/var/mobile/Applications" \
                                    << "/private/var/mobile/Containers/Bundle/Application" \
                                    << "/private/var/mobile/Applications" \
                                    << "/Applications");

    if (m_ESModsFolder.isEmpty())
    {
        QMessageBox::critical(NULL, tr("Error"), tr("Can't find Everlasting Summer installation folder\n") + iosFolderTrace);
        QApplication::quit();
    }

    QMessageBox::information(NULL, tr("Everlasting Summer"), tr("Mods are located in [") + m_ESModsFolder + "]\n" + iosFolderTrace);
    QMessageBox::information(NULL, tr("Everlasting Summer"), tr("My MAC-address is [") + AsyncDownloader::getMacAddress() + "]");
#elif defined(ANDROID)
    m_ESModsFolder = ANDROID_ES_MODS_FOLDER;
#else
    m_ESModsFolder = QDir::homePath() + "/tmp/su.sovietgames.everlasting_summer/files/";
#endif

    QNetworkReply *rep = AsyncDownloader::NetworkManager->get(QNetworkRequest(QUrl(QString(ES_MOD_INDEX_SERVER) + ES_MOD_INDEX_NAME)));
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

    case InfoUriRole:
        return element->infouri;
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
    roles[InfoUriRole] = "infouri";
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
    rep->deleteLater();

    m_JsonWriter.start();

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
                ESModElement *el = new ESModElement(ES_MOD_INDEX_SERVER, m_ESModsFolder, this);
                if (el->DeserializeFromNetwork(arr[i].toObject()))
                {
                    el->TryToPickupFrom(local_elements);
                    addModElement(el);
                }
                else
                {
                    delete el;
                }
            }
        }
    }

    foreach (ESModElement *el, local_elements)
        addModElement(el);

    sortList(m_lastSortMode);

    foreach (ESModElement *el, m_elements)
        el->RequestHeaders();

    emit esIndexReceived();

    SaveLocalModsDB();
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
    bool migrateFlag = false;
    QFile f(AsyncJsonWriter::configFileName());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Backward compatibility for old versions
        f.unsetError();
        f.setFileName(QDir(m_ESModsFolder).filePath(".esmanager_installed.db"));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        migrateFlag = true;
    }

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

    m_helpText = obj["helptext"].toString();
    if (!m_helpText.isEmpty())
        emit appHelpReceived(m_helpText, false);

    m_ESModsFolder = obj["modsfolder"].toString(m_ESModsFolder);

    QJsonArray arr = obj["packs"].toArray();
    for (int i = 0; i < arr.size(); ++i)
    {
        ESModElement *el = new ESModElement(ES_MOD_INDEX_SERVER, m_ESModsFolder, this);
        el->DeserializeFromDB(arr[i].toObject());
        l << el;
    }

    if (migrateFlag)
    {
        // Remove old config and directory if empty
        f.remove();
        QDir().rmpath(QFileInfo(f).dir().path());
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
    obj->insert("helptext", m_helpText);
    obj->insert("packs", arr);
    obj->insert("modsfolder", m_ESModsFolder);
    m_JsonWriter.write(obj);
}

static const char *statusNamesArr[] = { \
    "окончен", \
    "в разработке", \
    "заморожен", \
    "демо", \
    "надстройка", \
    "обучаловка" \
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
    if (a->title == b->title)
        return lessThanAsServer(a, b);

    return a->title < b->title;
}

static bool lessThanByName1(ESModElement *a, ESModElement *b)
{
    if (a->title == b->title)
        return lessThanAsServer(a, b);

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

    if (sz1 == sz2)
        return lessThanAsServer(a, b);

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

    if (sz1 == sz2)
        return lessThanAsServer(a, b);

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

    if (tm1 == tm2)
        return lessThanAsServer(a, b);

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

    if (tm1 == tm2)
        return lessThanAsServer(a, b);

    return tm1 >= tm2;
}

static float calcFiveScore(ESModElement *el)
{
    if (el->likemarkscount <= 0 && el->dislikemarkscount <= 0)
        return 0;

    if (el->likemarkscount <= 0)
        return 1.0;

    return 1.0 + el->likemarkscount * 4.5 / (el->likemarkscount + el->dislikemarkscount);
}

static bool lessThanByVotesCount(ESModElement *a, ESModElement *b)
{
    int vca = a->likemarkscount + a->dislikemarkscount;
    int vcb = b->likemarkscount + b->dislikemarkscount;

    if (vca == vcb)
        return lessThanAsServer(a, b);

    return vca >= vcb;
}

static bool lessThanByScore(ESModElement *a, ESModElement *b)
{
    float sa = calcFiveScore(a);
    float sb = calcFiveScore(b);

    if (sa == sb)
        return lessThanByVotesCount(a, b);

    return sa >= sb;
}

typedef bool (*lessThanSortFunc)(ESModElement *a, ESModElement *b);
static const lessThanSortFunc lessThanArray[] = { \
    lessThanAsServer, \
    lessThanByName0, \
    lessThanByName1, \
    lessThanBySize0, \
    lessThanBySize1, \
    lessThanByDate0, \
    lessThanByDate1, \
    lessThanByScore, \
    lessThanByVotesCount \
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

void ESModModel::helpRead(QString str)
{
    m_helpText = str;
    SaveLocalModsDB();
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
        iosFolderTrace += dir + "\n";
        QFileInfoList uuidlist = QDir(dir).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach (QFileInfo fiuuid, uuidlist) // Application UUIDs directories
        {
            iosFolderTrace += "  " + fiuuid.filePath() + "\n";
            QFileInfoList applist = QDir(fiuuid.filePath()).entryInfoList(QStringList("*.app"), QDir::Dirs | QDir::NoDotAndDotDot);
            foreach (QFileInfo fiapp, applist) // *.app directories
            {
                iosFolderTrace += "    " + fiapp.filePath() + "\n";
                QFileInfo iplist(fiapp.filePath() + "/Info.plist");
                if (!iplist.isFile())
                    continue;

                iosFolderTrace += "      Info.plist\n";

                QString bunid = QSettings(iplist.absoluteFilePath(), QSettings::NativeFormat).value("CFBundleIdentifier").toString();
                iosFolderTrace += "      " + bunid + "\n";
                if (bunid != "com.mifki.everlastingsummer")
                    continue;

                iosFolderTrace += "      FOUND!\n";
                return fiapp.filePath() + "/scripts/game/";
            }
        }
    }

    return QString();
}
#endif
