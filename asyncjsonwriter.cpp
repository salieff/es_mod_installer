#include <QJsonDocument>
#include <QFile>

#include <QDir>
#include "asyncjsonwriter.h"

AsyncJsonWriter::AsyncJsonWriter(QObject *parent)
    : QThread(parent),
      m_jsonObject(NULL),
      m_closeFlag(false)
{
}

AsyncJsonWriter::~AsyncJsonWriter()
{
    delete m_jsonObject;
}

void AsyncJsonWriter::close()
{
    QMutexLocker ml(&m_jsonMutex);
    m_closeFlag = true;
    m_jsonCondition.wakeAll();
}

void AsyncJsonWriter::write(QJsonObject *obj)
{
    QMutexLocker ml(&m_jsonMutex);
    delete m_jsonObject;
    m_jsonObject = obj;
    m_jsonCondition.wakeAll();
}

void AsyncJsonWriter::run()
{
    while (true)
    {
        QJsonObject *obj = NULL;

        m_jsonMutex.lock();
        obj = m_jsonObject;
        m_jsonObject = NULL;

        if (obj == NULL)
        {
            if (m_closeFlag)
            {
                m_jsonMutex.unlock();
                break;
            }

            m_jsonCondition.wait(&m_jsonMutex);
            obj = m_jsonObject;
            m_jsonObject = NULL;
        }
        m_jsonMutex.unlock();

        if (obj == NULL)
            continue;

        QJsonDocument *doc = new QJsonDocument(*obj);
        QByteArray data = doc->toJson();
        delete doc;
        delete obj;

#ifndef ANDROID
        QFile f(QString(ES_MOD_DB_PATH).replace(QRegExp("^/sdcard/Android/data"), QDir::homePath() + "/tmp"));
#else
        QFile f(ES_MOD_DB_PATH);
#endif

        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            continue;

        f.write(data);
        f.close();
    }
}
