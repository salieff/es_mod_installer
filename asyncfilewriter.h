#ifndef ASYNCFILEWRITER_H
#define ASYNCFILEWRITER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QByteArray>
#include <QFile>

#include <atomic>

class AsyncFileWriter : public QThread
{
    Q_OBJECT
public:
    AsyncFileWriter(QObject * parent = 0);
    virtual ~AsyncFileWriter();

    bool open(QString &destdir, QString &fname, QIODevice::OpenMode mode = QIODevice::WriteOnly);

    void write(QIODevice *dev);
    void close(bool abort = false);
    bool seek(qint64 pos);
    bool failed();
    bool aborted();
    QString errorString();
    void reset();

protected:
    virtual void run();

private:
    QFile m_file;
    QByteArray m_buffer;
    QMutex m_bufferMutex;
    QWaitCondition m_bufferCondition;

    std::atomic_bool m_closeFlag;
    std::atomic_bool m_wasError;
    std::atomic_bool m_wasAbort;
    QString m_errorString;
};

#endif // ASYNCFILEWRITER_H
