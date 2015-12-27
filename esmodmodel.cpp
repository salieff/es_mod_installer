#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QNetworkProxy>

#include "esmodmodel.h"

#define ES_MOD_INDEX_URL "http://191.ru/es/project1.json"

ESModModel::ESModModel(QObject *parent)
    : QAbstractListModel(parent),
      m_NetMgr(this),
      m_JsonWriter(this),
      m_busyIndicator(NULL),
      m_appTitleText(NULL),
      m_helpText(NULL)
{
#ifndef ANDROID
    // m_NetMgr.setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 3128));
#endif

    m_JsonWriter.start();

    QNetworkReply *rep = m_NetMgr.get(QNetworkRequest(QUrl(ES_MOD_INDEX_URL)));
    connect(rep, SIGNAL(finished()), this, SLOT(ESModIndexDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(ESModIndexError(QNetworkReply::NetworkError)));
}

ESModModel::~ESModModel()
{
    SaveLocalModsDB();
    m_JsonWriter.close();
    m_JsonWriter.wait();
}

void ESModModel::setBusyIndicator(QObject *bus)
{
    m_busyIndicator = bus;

    if (m_busyIndicator)
        m_busyIndicator->setProperty("running", true);
}

void ESModModel::setAppTitleText(QObject *txt)
{
    m_appTitleText = txt;
}

void ESModModel::setHelpText(QObject *txt)
{
    m_helpText = txt;
}

void ESModModel::addModElement(ESModElement *element)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_initialElements << element;
    m_elements << element;
    connect(element, SIGNAL(stateChanged()), this, SLOT(elementChanged()));
    connect(element, SIGNAL(saveMe()), this, SLOT(SaveLocalModsDB()));
    connect(element, SIGNAL(removeMe()), this, SLOT(elementNeedRemove()));
    endInsertRows();
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

    case PathRole:
        return element->path;
        break;

    case FilesRole:
        return element->files.join("\n");
        break;

    case StateRole:
        return element->StateName();
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
    roles[PathRole] = "path";
    roles[FilesRole] = "files";
    roles[StateRole] = "modstate";
    roles[ProgressRole] = "progress";
    roles[SizeRole] = "modsize";
    roles[TimestampRole] = "timestamp";
    roles[GuiBlockedRole] = "guiblocked";

    return roles;
}

void ESModModel::ESModIndexDownloaded()
{
    int serverIndex = 0;

    QList<ESModElement *> local_elements;
    LoadLocalModsDB(local_elements);

    if (m_busyIndicator)
        m_busyIndicator->setProperty("running", false);

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
            if (m_appTitleText != NULL && !appTitle.isEmpty())
                m_appTitleText->setProperty("text", appTitle);

            QString appHelp = obj["appReadMe"].toString();
            if (m_helpText != NULL && !appHelp.isEmpty())
                m_helpText->setProperty("text", appHelp);

            QJsonArray arr = obj["packs"].toArray();
            for (int i = 0; i < arr.size(); ++i)
            {
                ESModElement *el = new ESModElement(this);

                el->title = arr[i].toObject()["title"].toString().trimmed();
                el->status = arr[i].toObject()["status"].toString().trimmed();
                el->langs = arr[i].toObject()["lang"].toString().trimmed().split(QRegExp("[,\\s]+"), QString::SkipEmptyParts);
                el->uri = arr[i].toObject()["uri"].toString().trimmed();
                el->path = arr[i].toObject()["path"].toString().trimmed();
#ifndef ANDROID
                el->path.replace(QRegExp("^/sdcard/Android/data"), QDir::homePath() + "/tmp");
#endif

                QJsonArray files_arr = arr[i].toObject()["files"].toArray();
                for (int j = 0; j < files_arr.size(); ++j)
                    el->files << files_arr[j].toString().trimmed();

                for (int j = 0; j < local_elements.size(); ++j)
                {
                    QString title1 = local_elements[j]->title;
                    title1 = title1.remove(QRegExp("\\(\\b(?:Ru|Eng|Spa|,)\\b\\)")).remove(QRegExp("\\[.*\\]")).remove(QRegExp("\\{.*\\}")).simplified();
                    // printf("title1 = [%s]\n", title1.toLocal8Bit().data());

                    QString title2 = el->title;
                    title2 = title2.remove(QRegExp("\\(\\b(?:Ru|Eng|Spa|,)\\b\\)")).remove(QRegExp("\\[.*\\]")).remove(QRegExp("\\{.*\\}")).simplified();
                    // printf("title2 = [%s]\n", title2.toLocal8Bit().data());

                    if (title1 == title2)
                    {
                        el->m_localFiles = local_elements[j]->m_localFiles;
                        el->m_localSize = local_elements[j]->m_localSize;
                        el->m_localTimestamp = local_elements[j]->m_localTimestamp;
                        delete local_elements[j];
                        local_elements.removeAt(j);
                    }
                }

                el->m_serverIndex = serverIndex++;
                addModElement(el);
            }
        }
    }

    for (int j = 0; j < local_elements.size(); ++j)
    {
        local_elements[j]->m_serverIndex = serverIndex++;
        addModElement(local_elements[j]);
    }

    foreach (ESModElement *el, m_elements)
        el->RequestHeaders();

    rep->deleteLater();
}

void ESModModel::ESModIndexError(QNetworkReply::NetworkError code)
{
    Q_UNUSED(code);

    if (m_busyIndicator)
        m_busyIndicator->setProperty("running", false);

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

void ESModModel::elementChanged(int ind)
{
    if (ind < 0)
    {
        ESModElement *el = dynamic_cast<ESModElement *>(sender());
        for (int i = 0; i < m_elements.count(); ++i)
            if (el == m_elements[i])
            {
                ind = i;
                break;
            }
    }

    if (ind >= 0)
        emit dataChanged(index(ind, 0), index(ind, 0));
}

void ESModModel::elementNeedRemove()
{
    ESModElement *el = dynamic_cast<ESModElement *>(sender());
    for (int i = 0; i < m_elements.count(); ++i)
        if (el == m_elements[i])
        {
            beginRemoveRows(QModelIndex(), i, i);
            m_elements.removeAt(i);
            endRemoveRows();

            break;
        }

    for (int i = 0; i < m_initialElements.count(); ++i)
        if (el == m_initialElements[i])
        {
            m_initialElements.removeAt(i);
            break;
        }
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
    obj->insert("packs", arr);
    m_JsonWriter.write(obj);
}

static bool lessThanAsServer(ESModElement *a, ESModElement *b)
{
    return a->m_serverIndex < b->m_serverIndex;
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

void ESModModel::sortAsServer()
{
    beginResetModel();
    m_elements = m_initialElements;
    qSort(m_elements.begin(), m_elements.end(), lessThanAsServer);
    endResetModel();
}

void ESModModel::sortByName(int updown)
{
    beginResetModel();
    m_elements = m_initialElements;
    if (updown == 0)
        qSort(m_elements.begin(), m_elements.end(), lessThanByName0);
    else
        qSort(m_elements.begin(), m_elements.end(), lessThanByName1);
    endResetModel();
}

void ESModModel::sortBySize(int updown)
{
    beginResetModel();
    m_elements = m_initialElements;
    if (updown == 0)
        qSort(m_elements.begin(), m_elements.end(), lessThanBySize0);
    else
        qSort(m_elements.begin(), m_elements.end(), lessThanBySize1);
    endResetModel();
}

void ESModModel::sortByDate(int updown)
{
    beginResetModel();
    m_elements = m_initialElements;
    if (updown == 0)
        qSort(m_elements.begin(), m_elements.end(), lessThanByDate0);
    else
        qSort(m_elements.begin(), m_elements.end(), lessThanByDate1);
    endResetModel();
}

static bool lessThanKeyword(ESModElement *a, ESModElement *b)
{
    return a->m_keywordFilterCounter >= b->m_keywordFilterCounter;
}

void ESModModel::filterByKeywords(QString str)
{
    QStringList strList = str.trimmed().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QStringList::iterator sit = strList.begin();
    while (sit != strList.end())
    {
        if (sit->length() < 3)
            sit = strList.erase(sit);
        else
            ++sit;
    }

    beginResetModel();
    m_elements = m_initialElements;

    if (!strList.empty())
    {
        QList<ESModElement *>::iterator it = m_elements.begin();
        while (it != m_elements.end())
        {
            (*it)->m_keywordFilterCounter = 0;
            bool found = false;
            for (int i = 0; i < strList.size(); ++i)
                if ((*it)->title.contains(strList[i], Qt::CaseInsensitive))
                {
                    found = true;
                    (*it)->m_keywordFilterCounter += strList.size() - i;
                }

            if (found)
            {
                ++it;
                continue;
            }

            it = m_elements.erase(it);
        }

        qSort(m_elements.begin(), m_elements.end(), lessThanKeyword);
    }

    endResetModel();
}
