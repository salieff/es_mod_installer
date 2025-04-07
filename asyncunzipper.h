#ifndef ASYNCUNZIPPER_H
#define ASYNCUNZIPPER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "minizip/unzip.h"


class AsyncUnzipper : public QThread
{
    Q_OBJECT
public:
    explicit AsyncUnzipper(QObject * parent = 0);

    bool unzip(QString zipfile);
    bool aborted();
    bool failed();
    QString errorString();
    QStringList unpackedFiles();

    void setOverwriteFlags(bool ovrw, bool ovrw_always);

signals:
    void progress(int p);
    void overwriteRequest(QString fname);

public slots:
    void abort();

protected:
    virtual void run();

private:
    bool unpackZip(bool calcSizeOnly = UNPACK_ZIP);
    bool saveCurrentUnpFile(unzFile ufd, QString fname);
    bool checkOverwrite(QString fname);

    QString m_zipFile;
    qint64 m_totalSize;
    qint64 m_unpackedSize;
    int m_progress;
    QStringList m_unpackedFiles;

    bool m_abortFlag;
    bool m_failedFlag;
    QString m_errorString;
    QMutex m_abortMutex;

    bool m_canOverwrite;
    bool m_alwaysOverwrite;
    QMutex m_overwriteMutex;
    QWaitCondition m_overwriteCondition;

    constexpr static bool CALC_SIZE_ONLY = true;
    constexpr static bool UNPACK_ZIP = false;
};

#endif // ASYNCUNZIPPER_H
