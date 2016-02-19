#include <QDir>
#include "asyncfilewriter.h"

AsyncFileWriter::AsyncFileWriter(QObject * parent)
    : QThread(parent),
      m_closeFlag(false),
      m_wasError(false),
      m_wasAbort(false)
{

}

AsyncFileWriter::~AsyncFileWriter()
{

}

bool AsyncFileWriter::open(QString &destdir, QString &fname)
{
    m_closeFlag = false;
    m_wasError = false;
    m_wasAbort = false;
    m_errorString.clear();
    m_buffer.clear();
    m_file.unsetError();

    if (!QDir().mkpath(QFileInfo(destdir + fname).dir().path()))
    {
        m_errorString = tr("Can't create directory ") + QFileInfo(destdir + fname).dir().path();
        m_wasError = true;
        return false;
    }

    m_file.setFileName(destdir + fname);
    if (!m_file.open(QIODevice::WriteOnly))
    {
        m_wasError = true;
        m_errorString = m_file.fileName() + " : " + m_file.errorString();
        return false;
    }

    start();

    return true;
}

void AsyncFileWriter::write(QIODevice *dev)
{
    QByteArray data = dev->readAll();

    if (data.size() > 0)
    {
        QMutexLocker ml(&m_bufferMutex);

        m_buffer.push_back(data);
        m_bufferCondition.wakeAll();
    }
}

void AsyncFileWriter::close(bool abort)
{
    QMutexLocker ml(&m_bufferMutex);
    m_closeFlag = true;
    if (abort)
    {
        m_wasAbort = true;
        m_errorString = m_file.fileName() + " : " + tr("Aborted by user");
    }
    m_bufferCondition.wakeAll();
}

bool AsyncFileWriter::failed()
{
    return m_wasError;
}

bool AsyncFileWriter::aborted()
{
    return m_wasAbort;
}

QString AsyncFileWriter::errorString()
{
    return m_errorString;
}

void AsyncFileWriter::run()
{
    while (true)
    {
        QByteArray data;

        m_bufferMutex.lock();
        data.swap(m_buffer);

        if (data.isEmpty())
        {
            if (m_closeFlag)
            {
                m_bufferMutex.unlock();
                break;
            }

            m_bufferCondition.wait(&m_bufferMutex);
            data.swap(m_buffer);
        }
        m_bufferMutex.unlock();

        if (data.isEmpty())
            continue;

        if (m_file.write(data) != data.size())
        {
            m_wasError = true;
            m_errorString = m_file.fileName() + " : " + m_file.errorString();
            break;
        }
    }

    m_file.close();
}
