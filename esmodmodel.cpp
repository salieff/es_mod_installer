#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#include "esmodmodel.h"

#define ES_MOD_INDEX_URL "http://191.ru/es/project.json"
#define ES_MOD_DB_PATH "/sdcard/Android/data/su.sovietgames.everlasting_summer/files/.esmanager_installed.db"

ESModModel::ESModModel(QObject *parent)
    : QAbstractListModel(parent),
      m_NetMgr(this),
      m_busyIndicator(NULL),
      m_appTitleText(NULL)
{
#ifndef ANDROID
    // m_NetMgr.setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "127.0.0.1", 3128));
#endif
    QNetworkReply *rep = m_NetMgr.get(QNetworkRequest(QUrl(ES_MOD_INDEX_URL)));
    connect(rep, SIGNAL(finished()), this, SLOT(ESModIndexDownloaded()));
    connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(ESModIndexError(QNetworkReply::NetworkError)));
}

ESModModel::~ESModModel()
{
    SaveLocalModsDB();
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

void ESModModel::addModElement(ESModElement *element)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
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

            QJsonArray arr = obj["packs"].toArray();
            for (int i = 0; i < arr.size(); ++i)
            {
                ESModElement *el = new ESModElement(this);

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
