#include "statisticsmanager.h"
#include "asyncdownloader.h"

#include <QDateTime>

StatisticsManager * StatisticsManager::m_internalInstance = NULL;

StatisticsManager::StatisticsManager(QObject *parent) : QObject(parent)
{
}

StatisticsManager * StatisticsManager::getInstance(QObject *parent)
{
    if (m_internalInstance == NULL)
        m_internalInstance = new StatisticsManager(parent);

    return m_internalInstance;
}

void StatisticsManager::destroyInstance()
{
    delete m_internalInstance;
}

void StatisticsManager::addRequest(const QString &reqStr)
{
    m_requestQueue << QString("%1&time=%2").arg(reqStr).arg(QDateTime::currentMSecsSinceEpoch() / 1000);
    QNetworkReply *statRep = AsyncDownloader::get(reqStr);
    connect(statRep, SIGNAL(finished()), this, SLOT(requestFinished()));
}

void StatisticsManager::deserializeFromJSON(const QJsonArray &reqArr)
{
    m_requestQueue.clear();
    for (int i = 0; i < reqArr.size(); ++i)
        addRequest(reqArr[i].toString());
}

QJsonArray StatisticsManager::serializeToJSON()
{
    return QJsonArray::fromStringList(m_requestQueue);
}

void StatisticsManager::requestFinished()
{
    QNetworkReply *rep = dynamic_cast<QNetworkReply *>(sender());
    rep->deleteLater();

    if (rep->error() == QNetworkReply::NoError)
    {
        QString reqStr = rep->request().url().toString(QUrl::None);
        QStringList::iterator it = m_requestQueue.begin();
        while (it != m_requestQueue.end())
        {
            if ((*it).startsWith(reqStr, Qt::CaseInsensitive))
            {
                it = m_requestQueue.erase(it);
                break;
            }

            ++it;
        }
    }
    else
    {
        emit saveMe();
    }
}
