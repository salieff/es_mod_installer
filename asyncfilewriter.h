#ifndef ASYNCFILEWRITER_H
#define ASYNCFILEWRITER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QByteArray>
#include <QFile>

class AsyncFileWriter : public QThread
{
    Q_OBJECT
public:
    AsyncFileWriter(QObject * parent = 0);
    virtual ~AsyncFileWriter();

    bool open(QString &destdir, QString &fname);
    void write(QIODevice *dev);
    void close(bool abort = false);

    bool failed();
    bool aborted();
    QString errorString();

protected:
    virtual void run();

private:
    QFile m_file;
    QByteArray m_buffer;
    QMutex m_bufferMutex;
    QWaitCondition m_bufferCondition;

    bool m_closeFlag;
    bool m_wasError;
    bool m_wasAbort;
    QString m_errorString;
};

#endif // ASYNCFILEWRITER_H