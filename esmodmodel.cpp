#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QNetworkProxy>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QClipboard>
#include <QProcessEnvironment>
#include <QTimer>
#include <QAndroidJniObject>
#include <QtAndroid>

#include "version.h"
#include "esmodmodel.h"
#include "statisticsmanager.h"
#include "safadapter.h"

#define ES_MOD_INDEX_SERVER "http://191.ru/es/"
#define ES_MOD_INDEX_NAME "project2.json"


ESModModel::ESModModel(QObject *parent)
    : QAbstractListModel(parent),
      m_JsonWriter(this)
{
    QNetworkReply *rep = AsyncDownloader::get(ES_MOD_INDEX_SERVER, ES_MOD_INDEX_NAME);
    connect(rep, SIGNAL(finished()), this, SLOT(ESModIndexDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(ESModIndexError(QNetworkReply::NetworkError)));

    connect(StatisticsManager::getInstance(), SIGNAL(saveMe()), this, SLOT(SaveLocalModsDB()));
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

    ESModElement *element = m_elements[index.row()];

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
            return element->LocalSize();

        return element->size;
        break;

    case TimestampRole:
    {
        QDateTime dt;

        if (element->timestamp == 0)
            dt.setTime_t(element->LocalTimeStamp());
        else
            dt.setTime_t(element->timestamp);

        return dt.toString("yyyy.MM.dd");
    }
        break;

    case FavoriteRole:
        return element->favorite;
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

    case InstallsTotalRole :
        return element->insttotal;
        break;

    case InstallsActiveRole :
        return element->instactive;
        break;

    case InstallsTotalMonthRole :
        return element->insttotalmonth;
        break;

    case InstallsActiveMonthRole :
        return element->instactivemonth;
        break;

    case InstallsTotalWeekRole :
        return element->insttotalweek;
        break;

    case InstallsActiveWeekRole :
        return element->instactiveweek;
        break;

    case LifeTimeAVGRole :
        return element->lifetimeavg;
        break;

    case LifeTimeMAXRole :
        return element->lifetimemax;
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
    roles[FavoriteRole] = "favorite";
    roles[GuiBlockedRole] = "guiblocked";
    roles[MyLikeMarkRole] = "mylikemark";
    roles[LikeMarksCountRole] = "likemarkscount";
    roles[DislikeMarksCountRole] = "dislikemarkscount";
    roles[InstallsTotalRole] = "insttotal";
    roles[InstallsActiveRole] = "instactive";
    roles[InstallsTotalMonthRole] = "insttotalmonth";
    roles[InstallsActiveMonthRole] = "instactivemonth";
    roles[InstallsTotalWeekRole] = "insttotalweek";
    roles[InstallsActiveWeekRole] = "instactiveweek";
    roles[LifeTimeAVGRole] = "lifetimeavg";
    roles[LifeTimeMAXRole] = "lifetimemax";

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

    // Пусть последние добавленные будут сверху
    std::reverse(m_initialElements.begin(), m_initialElements.end());

    for(const auto &el: local_elements)
        addModElement(el);

    sortList(m_lastSortMode);

    requestAllLikes();
    requestAllStatistics();

    foreach (ESModElement *el, m_elements)
        el->RequestHeaders();

    emit esIndexReceived();

    SaveLocalModsDB();

    if (m_needShowHelp)
        QTimer::singleShot(1000, this, SLOT(showDefferedHelp()));

    emit appTitleReceived(QString("Version %1.%2-%3 (%4 mods)")
                          .arg(ESM_VERSION_MAJOR)
                          .arg(ESM_VERSION_MINOR)
                          .arg(ESM_VERSION_BUILD)
                          .arg(m_initialElements.size()));
}

void ESModModel::ESModIndexError(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);

    sortList(m_lastSortMode);
    emit esIndexReceived();

    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    QMessageBox::critical(NULL, "Index download error", rep->errorString());
}

void ESModModel::AllLikesReceived()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    rep->deleteLater();

    if (rep->error() != QNetworkReply::NoError)
        return;

    QByteArray data = rep->readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (doc.isNull())
    {
        QMessageBox::critical(NULL, tr("Likes list isn't valid JSON"), err.errorString());
        return;
    }

    if (!doc.isObject())
    {
        QMessageBox::critical(NULL, tr("Bad likes list"), tr("Likes list doesn't contain any JSON objects"));
        return;
    }

    QJsonObject obj = doc.object();
    if (obj["result"].toString() != "ok")
    {
        QMessageBox::critical(NULL, tr("Likes list request failed"), obj["result"].toString());
        return;
    }

    QJsonArray arr = obj["marks"].toArray();
    beginResetModel();
    foreach (ESModElement *el, m_elements)
    {
        for (int i = 0; i < arr.size(); ++i)
            if (el->DeserializeFromAllLikesList(arr[i].toObject()))
                break;

        if (el->likemarkscount < 0)
            el->likemarkscount = 0;

        if (el->dislikemarkscount < 0)
            el->dislikemarkscount = 0;
    }

    endResetModel();
}

void ESModModel::AllStatisticsReceived()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    rep->deleteLater();

    if (rep->error() != QNetworkReply::NoError)
        return;

    QByteArray data = rep->readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (doc.isNull())
    {
        QMessageBox::critical(NULL, tr("Statistics list isn't valid JSON"), err.errorString());
        return;
    }

    if (!doc.isObject())
    {
        QMessageBox::critical(NULL, tr("Bad statistics list"), tr("Statistics list doesn't contain any JSON objects"));
        return;
    }

    QJsonObject obj = doc.object();
    if (obj["result"].toString() != "ok")
    {
        QMessageBox::critical(NULL, tr("Statistics list request failed"), obj["result"].toString());
        return;
    }

    QJsonArray arr = obj["statistics"].toArray();
    beginResetModel();
    for (int i = 0; i < arr.size(); ++i)
        foreach (ESModElement *el, m_elements)
            el->DeserializeFromAllStatisticsList(arr[i].toObject());
    endResetModel();
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

void ESModModel::ToggleFavorite(int ind)
{
    m_elements[ind]->ToggleFavorite();
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

void ESModModel::requestAllLikes()
{
    QString allLikeReq = QString("%1?operation=queryallmarks&udid=%2")\
            .arg(LIKES_CGI_URL)\
            .arg(AsyncDownloader::getDeviceUDID());
    QNetworkReply *allLikeRep = AsyncDownloader::get(allLikeReq);
    connect(allLikeRep, SIGNAL(finished()), this, SLOT(AllLikesReceived()));
}

void ESModModel::requestAllStatistics()
{
    QString allStatReq = QString("%1?operation=queryallstatistics").arg(STATS_CGI_URL);
    QNetworkReply *allStatRep = AsyncDownloader::get(allStatReq);
    connect(allStatRep, SIGNAL(finished()), this, SLOT(AllStatisticsReceived()));
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
        {
            m_needShowHelp = true;
            return false;
        }

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

    m_ModsInstallLocation = static_cast<ModsInstallLocation>(obj["mods_install_location"].toInt(ModsInstallLocationData));
    emit currentModsInstallLocation(m_ModsInstallLocation);

    if (m_ModsInstallLocation == ModsInstallLocationData)
        m_ESModsFolder = ANDROID_ES_MODS_FOLDER_DATA;
    else
        m_ESModsFolder = ANDROID_ES_MODS_FOLDER_MEDIA;

    // FIXME: Can't sort by size or date until headers weren't received
    // m_lastSortMode = (ESModModel::SortMode)obj["sortmode"].toInt();

    m_helpText = obj["helptext"].toString();
    if (!m_helpText.isEmpty())
        emit appHelpReceived(m_helpText, false);

    QJsonArray arr = obj["packs"].toArray();
    for (int i = 0; i < arr.size(); ++i)
    {
        ESModElement *el = new ESModElement(ES_MOD_INDEX_SERVER, m_ESModsFolder, this);
        el->DeserializeFromDB(arr[i].toObject());
        l << el;
    }

    StatisticsManager::getInstance()->deserializeFromJSON(obj["deferredStatistics"].toArray());

    QString ver = obj["version"].toString();
    if (ver != QString("%1.%2-%3").arg(ESM_VERSION_MAJOR).arg(ESM_VERSION_MINOR).arg(ESM_VERSION_BUILD))
        m_needShowHelp = true;

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
        if (!m_initialElements[i]->m_localFilesMap.empty() || m_initialElements[i]->favorite)
            arr.push_back(m_initialElements[i]->SerializeToDB());

    QJsonObject *obj = new QJsonObject;
    obj->insert("mods_install_location", (int)m_ModsInstallLocation);
    obj->insert("sortmode", (int)m_lastSortMode);
    obj->insert("helptext", m_helpText);
    obj->insert("packs", arr);
    obj->insert("deferredStatistics", StatisticsManager::getInstance()->serializeToJSON());
    obj->insert("version", QString("%1.%2-%3").arg(ESM_VERSION_MAJOR).arg(ESM_VERSION_MINOR).arg(ESM_VERSION_BUILD));
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

static bool lessThanByStatus(ESModElement *a, ESModElement *b)
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
        return lessThanByStatus(a, b);

    return a->title < b->title;
}

static bool lessThanByName1(ESModElement *a, ESModElement *b)
{
    if (a->title == b->title)
        return lessThanByStatus(a, b);

    return a->title >= b->title;
}

static bool lessThanBySize0(ESModElement *a, ESModElement *b)
{
    double sz1 = a->size;
    if (sz1 == 0)
        sz1 = a->LocalSize();

    double sz2 = b->size;
    if (sz2 == 0)
        sz2 = b->LocalSize();

    if (sz1 == sz2)
        return lessThanByStatus(a, b);

    return sz1 < sz2;
}

static bool lessThanBySize1(ESModElement *a, ESModElement *b)
{
    double sz1 = a->size;
    if (sz1 == 0)
        sz1 = a->LocalSize();

    double sz2 = b->size;
    if (sz2 == 0)
        sz2 = b->LocalSize();

    if (sz1 == sz2)
        return lessThanByStatus(a, b);

    return sz1 >= sz2;
}

static bool lessThanByDate0(ESModElement *a, ESModElement *b)
{
    double tm1 = a->timestamp;
    if (tm1 == 0)
        tm1 = a->LocalTimeStamp();

    double tm2 = b->timestamp;
    if (tm2 == 0)
        tm2 = b->LocalTimeStamp();

    if (tm1 == tm2)
        return lessThanByStatus(a, b);

    return tm1 < tm2;
}

static bool lessThanByDate1(ESModElement *a, ESModElement *b)
{
    double tm1 = a->timestamp;
    if (tm1 == 0)
        tm1 = a->LocalTimeStamp();

    double tm2 = b->timestamp;
    if (tm2 == 0)
        tm2 = b->LocalTimeStamp();

    if (tm1 == tm2)
        return lessThanByStatus(a, b);

    return tm1 >= tm2;
}

static int calcFiveScore(ESModElement *el)
{
    if (el->likemarkscount <= 0 && el->dislikemarkscount <= 0)
        return 0;

    int div1 = 0;
    int div2 = 0;

    if (el->likemarkscount > 0)
    {
        div1 = el->likemarkscount;
        div2 = el->likemarkscount;
    }

    if (el->dislikemarkscount > 0)
        div2 += el->dislikemarkscount;

    /*
    static const char *scoreStringsArr[] = { \
              "1", "1+", \
        "2-", "2", "2+", \
        "3-", "3", "3+", \
        "4-", "4", "4+", \
        "5-", "5", "5+" };

    size_t arrSize = sizeof(scoreStringsArr) / sizeof(scoreStringsArr[0]);
    */

    size_t arrSize = 14;
    return arrSize * div1 / (div2 + 1); // [0, arrSize)
}

static bool lessThanByVotesCount(ESModElement *a, ESModElement *b)
{
    int vca = a->likemarkscount + a->dislikemarkscount;
    int vcb = b->likemarkscount + b->dislikemarkscount;

    if (vca == vcb)
        return lessThanByStatus(a, b);

    return vca >= vcb;
}

static bool lessThanByScore(ESModElement *a, ESModElement *b)
{
    int sa = calcFiveScore(a);
    int sb = calcFiveScore(b);

    if (sa == sb)
        return lessThanByVotesCount(a, b);

    return sa >= sb;
}

static bool lessThanByActiveInstalls(ESModElement *a, ESModElement *b)
{
    if (a->instactive != b->instactive)
        return a->instactive > b->instactive;

    if (a->instactivemonth != b->instactivemonth)
        return a->instactivemonth > b->instactivemonth;

    if (a->instactiveweek != b->instactiveweek)
        return a->instactiveweek > b->instactiveweek;

    return lessThanByStatus(a, b);
}

static bool lessThanByTotalInstalls(ESModElement *a, ESModElement *b)
{
    if (a->insttotal != b->insttotal)
        return a->insttotal > b->insttotal;

    if (a->insttotalmonth != b->insttotalmonth)
        return a->insttotalmonth > b->insttotalmonth;

    if (a->insttotalweek != b->insttotalweek)
        return a->insttotalweek > b->insttotalweek;

    return lessThanByStatus(a, b);
}

static bool lessThanByLifeTime(ESModElement *a, ESModElement *b)
{
    if (a->lifetimeavg != b->lifetimeavg)
        return a->lifetimeavg > b->lifetimeavg;

    if (a->lifetimemax != b->lifetimemax)
        return a->lifetimemax > b->lifetimemax;

    return lessThanByStatus(a, b);
}

typedef bool (*lessThanSortFunc)(ESModElement *a, ESModElement *b);
static const lessThanSortFunc lessThanArray[] = { \
    nullptr, \
    lessThanByStatus, \
    lessThanByName0, \
    lessThanByName1, \
    lessThanBySize0, \
    lessThanBySize1, \
    lessThanByDate0, \
    lessThanByDate1, \
    lessThanByScore, \
    lessThanByVotesCount, \
    lessThanByActiveInstalls, \
    lessThanByTotalInstalls, \
    lessThanByLifeTime \
};

void ESModModel::sortList(SortMode m)
{
    m_lastSortMode = m;

    beginResetModel();
    m_elements = m_initialElements;
    if (lessThanArray[m])
        std::sort(m_elements.begin(), m_elements.end(), lessThanArray[m]);
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

    QStringList strList = str.trimmed().split(QRegExp("\\s+"), Qt::SkipEmptyParts);
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

    std::sort(m_elements.begin(), m_elements.end(), lessThanKeyword);
    ReindexElements();
    endResetModel();
}

void ESModModel::helpRead(QString str)
{
    m_helpText = str;
    SaveLocalModsDB();
}

void ESModModel::copyTraceback(bool forLog)
{
    QFile f;
    if (!SafAdapter::getAdapter("Android/data").OpenQFile(f, QDir(ANDROID_ES_MODS_FOLDER_DATA).filePath(forLog ? "log.txt" : "traceback.txt"), QIODevice::ReadOnly | QIODevice::Text))
        return;

    QDateTime modTime = QFileInfo(f).lastModified();
    QString s = QString("[") + modTime.toString("yyyy-MM-dd hh:mm:ss t") + QString("]\n");
    s += QString::fromLocal8Bit(f.readAll());
    copyToClipboard(s, QString(forLog ? tr("Log") : tr("Traceback")) + tr(" was copied into clipboard"));

    emit tracebackText(s);
}

void ESModModel::showDefferedHelp()
{
    emit showMeHelp("");
}

void ESModModel::ReindexElements()
{
    for (int i = 0; i < m_elements.size(); ++i)
        m_initialElements[i]->m_modelIndex = -1;

    for (int i = 0; i < m_elements.size(); ++i)
        m_elements[i]->m_modelIndex = i;
}

void ESModModel::copyToClipboard(const QString &txt, const QString &msg)
{
    QApplication::clipboard()->setText(txt);
    emit balloonText(msg);
}

void ESModModel::setModsInstallLocation(ModsInstallLocation location)
{
    m_ModsInstallLocation = location;

    if (m_ModsInstallLocation == ModsInstallLocationData)
    {
        m_ESModsFolder = ANDROID_ES_MODS_FOLDER_DATA;
        SafAdapter::setCurrentAdapter("Android/data");
    }
    else
    {
        m_ESModsFolder = ANDROID_ES_MODS_FOLDER_MEDIA;
        SafAdapter::setCurrentAdapter("Android/media");
    }

    QTimer::singleShot(100, [this]() {
        for (ESModElement *el: m_initialElements)
        {
            el->SetInstallPath(m_ESModsFolder);
            el->state = ESModElement::Unknown;
            el->headersReceived();
        }

        SaveLocalModsDB();
        emit currentModsInstallLocation(m_ModsInstallLocation);
    });
}
