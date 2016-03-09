#ifndef ASYNCDOWNLOADER_H
#define ASYNCDOWNLOADER_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QMutex>
#include <QWaitCondition>

#include "asyncfilewriter.h"

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
    QString errorString();
    QStringList downloadedFiles();
    void getHeadersData(double &sz, double &tm);

    static void createNetworkManager(QObject *parent = NULL);
    static QString getMacAddress();
    static QString getDeviceUDID();
    static QNetworkReply * get(QString url);
    static QNetworkReply * get(QString baseUrl, QString fileUrl);
    static QNetworkReply * get(QUrl url);
    static QNetworkReply * head(QString url);
    static QNetworkReply * head(QString baseUrl, QString fileUrl);
    static QNetworkReply * head(QUrl url);

signals:
    void progress(int);
    void finished();
    void headersReady();

// private signals:
    void abortDownload();

public slots:
    void abort();

private slots:
    void fileDownloaded();
    void fileWritten();
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
    QString m_errorString;

    QStringList m_localFiles;
    double m_size;
    double m_timestamp;

    AsyncFileWriter m_file;

    bool m_alwaysOverwrite;

    static QNetworkAccessManager *m_networkManager;
    static QString m_myMacAddress;
    static QString m_myUDID;
};

#endif // ASYNCDOWNLOADER_H
