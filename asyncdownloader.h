#ifndef ASYNCDOWNLOADER_H
#define ASYNCDOWNLOADER_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMutex>
#include <QWaitCondition>

class AsyncDownloader : public QObject
{
    Q_OBJECT
public:
    explicit AsyncDownloader(QObject *parent = 0);
    virtual ~AsyncDownloader();

    bool downloadFileList(QString url, QStringList &files, QString destdir, bool headers_only = false);
    bool wait(unsigned long t = ULONG_MAX);
    bool aborted();
    bool failed();
    QStringList downloadedFiles();
    void getHeadersData(double &sz, double &tm);

    void setOverwriteFlags(bool ovrw, bool ovrw_always);

signals:
    void progress(int);
    void finished();
    void headersReady();
    void overwriteRequest(QString fname);

// private signals:
    void abortDownload();

public slots:
    void abort();

private slots:
    void download();
    void fileDownloaded();
    void downloadProgress(qint64, qint64);
    void readData();

private:
    bool checkOverwrite(QString fname);

    bool m_headersOnly;
    QString m_url;
    QStringList m_files;
    QString m_destDir;
    int m_currFileIndex;
    int m_progress;
    bool m_wasError;
    bool m_wasAbort;

    QStringList m_localFiles;
    double m_size;
    double m_timestamp;

    QThread m_thread;
    QNetworkAccessManager m_netMgr;
    QFile m_file;

    bool m_canOverwrite;
    bool m_alwaysOverwrite;
    QMutex m_overwriteMutex;
    QWaitCondition m_overwriteCondition;

#ifdef ANDROID
//    QFile m_debugFile;
#endif
};

#endif // ASYNCDOWNLOADER_H
