#ifndef ASYNCFILEWRITER_H
#define ASYNCFILEWRITER_H

#include <QFile>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class AsyncFileWriter : public QThread
{
    Q_OBJECT
public:
    AsyncFileWriter(QObject * parent = 0);

    bool open(QString name);
    void abort();
    bool aborted();
    void fail();
    bool failed();
    void close();
    void write(QByteArray &arr);

signals:
    void error(QString s);

protected:
    virtual void run();

private:
    QByteArray m_buffer;
    QMutex m_bufferMutex;
    QWaitCondition m_bufferCondition;
    QFile m_file;
    bool m_closeFlag;
    bool m_abortFlag;
    bool m_failedFlag;
};

#endif // ASYNCFILEWRITER_H
