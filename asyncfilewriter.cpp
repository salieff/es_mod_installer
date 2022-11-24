#include <QDir>
#include "asyncfilewriter.h"
#include "safadapter.h"


static const int MAX_BUFFER_SIZE = 256 * 1024 * 1024;


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

bool AsyncFileWriter::open(QString &fname, QIODevice::OpenMode mode)
{
    reset();

    if (!SafAdapter::getCurrentAdapter().CreateQFile(m_file, fname, mode, SafAdapter::CREATE_FOLDERS))
    {
        m_wasError = true;
        m_errorString = tr("Can't create file ") + fname + " : " + m_file.errorString();
        return false;
    }

    start();

    return true;
}

void AsyncFileWriter::write(QIODevice *dev)
{
    QByteArray data = dev->readAll();
    if (data.isEmpty())
        return;

    QMutexLocker ml(&m_bufferMutex);

    while (m_buffer.size() > MAX_BUFFER_SIZE)
        m_bufferCondition.wait(&m_bufferMutex);

    m_buffer.push_back(data);
    m_bufferCondition.wakeAll();
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

bool AsyncFileWriter::seek(qint64 pos)
{
    return m_file.seek(pos);
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

void AsyncFileWriter::reset()
{
    m_closeFlag = false;
    m_wasError = false;
    m_wasAbort = false;
    m_errorString.clear();
    m_buffer.clear();
    m_file.unsetError();
}

void AsyncFileWriter::run()
{
    while (true)
    {
        QByteArray data;

        {
            QMutexLocker ml(&m_bufferMutex);

            while (m_buffer.isEmpty() && !m_closeFlag)
                m_bufferCondition.wait(&m_bufferMutex);

            if (m_closeFlag)
                break;

            data.swap(m_buffer);

            m_bufferCondition.wakeAll();
        }

        if (m_file.write(data) != data.size())
        {
            m_wasError = true;
            m_errorString = m_file.fileName() + " : " + m_file.errorString();
            break;
        }
    }

    m_file.close();
}
