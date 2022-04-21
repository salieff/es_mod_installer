#ifndef ASYNCUNZIPPER_H
#define ASYNCUNZIPPER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "minizip/unzip.h"
#include "modpaths.h"


class AsyncUnzipper : public QThread
{
    Q_OBJECT
public:
    explicit AsyncUnzipper(QObject * parent = 0, QString stopFolder = ANDROID_ES_MODS_FOLDER_DATA);

    bool unzipList(QStringList ziplist, QString destdir);
    bool aborted();
    bool failed();
    QString errorString();
    QStringList unpackedFiles();

    void setOverwriteFlags(bool ovrw, bool ovrw_always);
    void setStopFolder(QString stopFolder);

signals:
    void progress(int p);
    void overwriteRequest(QString fname);

public slots:
    void abort();

protected:
    virtual void run();

private:
    bool calculateTotalSize();
    bool unpackZip(QString zipFile, bool calcSizeOnly = false);
    bool saveCurrentUnpFile(unzFile ufd, QString fname);
    bool checkOverwrite(QString fname);

    QStringList m_zipList;
    QString m_destDir;
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

    QString m_parentStopFolder;
};

#endif // ASYNCUNZIPPER_H
