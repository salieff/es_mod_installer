#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#include "esmodmodel.h"

#define ES_MOD_INDEX_URL "https://raw.githubusercontent.com/Oziabr/ESmanager/master/project.json"
#define ES_MOD_DB_PATH "/sdcard/Android/data/su.sovietgames.everlasting_summer/files/.esmanager_installed.db"

ESModModel::ESModModel(QNetworkAccessManager *mgr, QObject *parent)
    : QAbstractListModel(parent),
      m_NetMgr(mgr)
{
    QNetworkReply *rep = m_NetMgr->get(QNetworkRequest(QUrl(ES_MOD_INDEX_URL)));
    connect(rep, SIGNAL(finished()), this, SLOT(ESModIndexDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(ESModIndexError(QNetworkReply::NetworkError)));
}

ESModModel::~ESModModel()
{
}

void ESModModel::setBusyIndicator(QObject *bus)
{
    m_busyIndicator = bus;

    if (m_busyIndicator)
        m_busyIndicator->setProperty("running", true);
}

void ESModModel::addModElement(ESModElement *element)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_elements << element;
    connect(element, SIGNAL(stateChanged()), this, SLOT(elementChanged()));
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
        return element->size;
        break;

    case TimestampRole:
        return element->timestamp;
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
    roles[UriRole] = "uri";
    roles[PathRole] = "path";
    roles[FilesRole] = "files";
    roles[StateRole] = "modstate";
    roles[ProgressRole] = "progress";
    roles[SizeRole] = "modsize";
    roles[TimestampRole] = "timestamp";
    return roles;
}

void ESModModel::ESModIndexDownloaded()
{
    QList<ESModElement *> local_elements;
    LoadLocalModsDB(local_elements);

    if (m_busyIndicator)
        m_busyIndicator->setProperty("running", false);

    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    if (rep->error() == QNetworkReply::NoError)
    {
        /*
        addModElement(new ESModElement(ESModElement::Available, 100, m_NetMgr, this));
        addModElement(new ESModElement(ESModElement::Downloading, 70, m_NetMgr, this));
        addModElement(new ESModElement(ESModElement::Unpacking, 70, m_NetMgr, this));
        addModElement(new ESModElement(ESModElement::Failed, 100, m_NetMgr, this));
        addModElement(new ESModElement(ESModElement::InstalledAvailable, 100, m_NetMgr, this));
        addModElement(new ESModElement(ESModElement::InstalledHasUpdate, 100, m_NetMgr, this));
        addModElement(new ESModElement(ESModElement::Installed, 100, m_NetMgr, this));
        */

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
            QJsonArray arr = obj["packs"].toArray();
            for (int i = 0; i < arr.size(); ++i)
            {
                ESModElement *el = new ESModElement(m_NetMgr, this);

                el->title = arr[i].toObject()["title"].toString().trimmed();
                el->uri = arr[i].toObject()["uri"].toString().trimmed();
                el->path = arr[i].toObject()["path"].toString().trimmed();
#ifndef ANDROID
                el->path.replace(QRegExp("^/sdcard/Android/data"), QDir::homePath() + "/tmp");
#endif

                QJsonArray files_arr = arr[i].toObject()["files"].toArray();
                for (int j = 0; j < files_arr.size(); ++j)
                    el->files << files_arr[j].toString().trimmed();

                for (int j = 0; j < local_elements.size(); ++j)
                    if (local_elements[j]->title == el->title)
                    {
                        el->m_localFiles = local_elements[j]->m_localFiles;
                        el->m_localSize = local_elements[j]->m_localSize;
                        el->m_localTimestamp = local_elements[j]->m_localTimestamp;
                        delete local_elements[j];
                        local_elements.removeAt(j);
                    }

                addModElement(el);
            }
        }
    }

    for (int j = 0; j < local_elements.size(); ++j)
        addModElement(local_elements[j]);

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
    // printf("[%s] %d\n", __PRETTY_FUNCTION__, ind);
    m_elements[ind]->Download();
}

void ESModModel::Abort(int ind)
{
    // printf("[%s] %d\n", __PRETTY_FUNCTION__, ind);
    m_elements[ind]->Abort();
}

void ESModModel::Retry(int ind)
{
    // printf("[%s] %d\n", __PRETTY_FUNCTION__, ind);
    m_elements[ind]->state = ESModElement::Available;
    m_elements[ind]->Download();
}

void ESModModel::Update(int ind)
{
    // printf("[%s] %d\n", __PRETTY_FUNCTION__, ind);
    m_elements[ind]->Update();
}

void ESModModel::Delete(int ind)
{
    // printf("[%s] %d\n", __PRETTY_FUNCTION__, ind);
    m_elements[ind]->Delete();
    if (m_elements[ind]->state == ESModElement::Installed)
    {
        beginRemoveRows(QModelIndex(), ind, ind);
        delete m_elements[ind];
        m_elements.removeAt(ind);
        endRemoveRows();
    }
    else
    {
        m_elements[ind]->state = ESModElement::Available;
        elementChanged(ind);
    }

    SaveLocalModsDB();
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
    {
        if (m_elements[ind]->state == ESModElement::Installed || \
                m_elements[ind]->state == ESModElement::InstalledAvailable || \
                m_elements[ind]->state == ESModElement::InstalledHasUpdate)
            SaveLocalModsDB();

        emit dataChanged(index(ind, 0), index(ind, 0));
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
        ESModElement *el = new ESModElement(m_NetMgr, this);
        el->DeserializeFromDB(arr[i].toObject());
        l << el;
    }

    return true;
}

void ESModModel::SaveLocalModsDB()
{
    QJsonArray arr;
    for (int i = 0; i < m_elements.size(); ++i)
        if (m_elements[i]->state == ESModElement::Installed || \
                m_elements[i]->state == ESModElement::InstalledAvailable || \
                m_elements[i]->state == ESModElement::InstalledHasUpdate)
            arr.push_back(m_elements[i]->SerializeToDB());

    QJsonObject root;
    root["packs"] = arr;

    QJsonDocument doc(root);
    QByteArray data = doc.toJson();

#ifndef ANDROID
    QFile f(QString(ES_MOD_DB_PATH).replace(QRegExp("^/sdcard/Android/data"), QDir::homePath() + "/tmp"));
#else
    QFile f(ES_MOD_DB_PATH);
#endif

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    f.write(data);
    f.close();
}
