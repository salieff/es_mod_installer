#ifndef ASYNCUNZIPPER_H
#define ASYNCUNZIPPER_H

#include <QThread>
#include <QMutex>

#include "minizip/unzip.h"

class AsyncUnzipper : public QThread
{
    Q_OBJECT
public:
    AsyncUnzipper(QObject * parent = 0);
    bool UnzipList(QStringList ziplist, QString destdir);
    void abort();
    bool aborted();

    QStringList getUnpackedFileList();

signals:
    void progress(int p);
    void error(QString s);

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
    QMutex m_abortMutex;
};

#endif // ASYNCUNZIPPER_H
