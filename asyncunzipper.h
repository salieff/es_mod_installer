#ifndef ASYNCUNZIPPER_H
#define ASYNCUNZIPPER_H

#include <QThread>
#include <QMutex>

#include "minizip/unzip.h"

class AsyncUnzipper : public QThread
{
    Q_OBJECT
public:
    explicit AsyncUnzipper(QObject * parent = 0);

    bool unzipList(QStringList ziplist, QString destdir);
    bool aborted();
    bool failed();
    QStringList unpackedFiles();

signals:
    void progress(int p);

public slots:
    void abort();

protected:
    virtual void run();

private:
    bool calculateTotalSize();
    bool unpackZip(QString zipFile);
    bool saveCurrentUnpFile(unzFile ufd, QString fname);

    QStringList m_zipList;
    QString m_destDir;
    qint64 m_totalSize;
    qint64 m_unpackedSize;
    int m_progress;
    QStringList m_unpackedFiles;

    bool m_abortFlag;
    bool m_failedFlag;
    QMutex m_abortMutex;
};

#endif // ASYNCUNZIPPER_H
