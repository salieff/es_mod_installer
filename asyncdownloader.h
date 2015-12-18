#ifndef ASYNCDOWNLOADER_H
#define ASYNCDOWNLOADER_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class AsyncDownloader : public QObject
{
    Q_OBJECT
public:
    explicit AsyncDownloader(QObject *parent = 0);

    bool downloadFileList(QString url, QStringList &files, QString destdir, bool headers_only = false);
    bool wait(unsigned long t = ULONG_MAX);
    bool aborted();
    bool failed();
    QStringList downloadedFiles();
    void getHeadersData(double &sz, double &tm);

signals:
    void progress(int);
    void finished();
    void headersReady();

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
    bool m_headersOnly;
    QString m_url;
    QStringList m_files;
    QString m_destDir;
    int m_currFileIndex;
    bool m_wasError;
    bool m_wasAbort;

    QStringList m_localFiles;
    double m_size;
    double m_timestamp;

    QThread m_thread;
    QNetworkAccessManager m_netMgr;
    QFile m_file;
};

#endif // ASYNCDOWNLOADER_H
