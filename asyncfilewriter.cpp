#include "asyncfilewriter.h"

AsyncFileWriter::AsyncFileWriter(QObject *parent)
    : QThread(parent),
      m_closeFlag(false),
      m_abortFlag(false),
      m_failedFlag(false)
{
}

bool AsyncFileWriter::open(QString name)
{
    m_bufferMutex.lock();
    m_closeFlag = false;
    m_abortFlag = false;
    m_failedFlag = false;
    m_file.setFileName(name);
    bool ret = m_file.open(QIODevice::WriteOnly);
    m_bufferMutex.unlock();

    if (ret)
        start();

    return ret;
}

void AsyncFileWriter::close()
{
    m_bufferMutex.lock();
    m_closeFlag = true;
    m_bufferCondition.wakeAll();
    m_bufferMutex.unlock();
}

void AsyncFileWriter::abort()
{
    m_bufferMutex.lock();
    m_closeFlag = true;
    m_abortFlag = true;
    m_buffer.clear();
    m_bufferCondition.wakeAll();
    m_bufferMutex.unlock();
}

bool AsyncFileWriter::aborted()
{
    bool ret;

    m_bufferMutex.lock();
    ret = m_abortFlag;
    m_bufferMutex.unlock();

    return ret;
}

void AsyncFileWriter::fail()
{
    m_bufferMutex.lock();
    m_failedFlag = true;
    m_bufferMutex.unlock();
}

bool AsyncFileWriter::failed()
{
    bool ret;

    m_bufferMutex.lock();
    ret = m_failedFlag;
    m_bufferMutex.unlock();

    return ret;
}

void AsyncFileWriter::write(QByteArray &arr)
{
    m_bufferMutex.lock();
    m_buffer.append(arr);
    m_bufferCondition.wakeAll();
    m_bufferMutex.unlock();
}

void AsyncFileWriter::run()
{
    while (true)
    {
        QByteArray arr;

        m_bufferMutex.lock();
        arr.swap(m_buffer);

        if (m_abortFlag || (arr.isEmpty() && m_closeFlag))
        {
            m_file.close();
            m_bufferMutex.unlock();
            break;
        }

        if (arr.isEmpty())
        {
            m_bufferCondition.wait(&m_bufferMutex);
            arr.swap(m_buffer);
        }
        m_bufferMutex.unlock();

        if (arr.isEmpty())
            continue;

        // printf("[%s] %d\n", __PRETTY_FUNCTION__, arr.size());
        if (m_file.write(arr) < 0)
        {
            emit error(m_file.errorString());
            fail();
            break;
        }
    }
}
